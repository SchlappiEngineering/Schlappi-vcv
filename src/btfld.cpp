#include "plugin.hpp"
#include "widgets/schlappi_widgets.hpp"
#include <iostream>
#include <cmath>
#include <math.h>
#include <array>
#include <vector>

#define NIBBLE 4
#define BTFLD_UPSAMPLE_RATE 8
#define BTFLD_UPSAMPLE_QUALITY 12

struct ACCouplingFilter {
    ACCouplingFilter() : xPrev(0), yPrev(0), scalar(0) {}

    void setDecay(float halflife) {
        scalar = std::pow(2, -1.f / halflife);
    }

    float process(float x) {
        auto y = scalar * (x + yPrev - xPrev);
        yPrev = y;
        xPrev = x;
        return y;
    }
public:
    float xPrev, yPrev, scalar;
};

struct BitCalculator {
    int stepSize;
    int delayBeforeGoingHigh = BTFLD_UPSAMPLE_RATE * 1.5f;
    int counter;
    int lastOddValue;

    BitCalculator() {
        counter = 0;
        lastOddValue = 0;
    }

    bool oddTracker(int input) {
        if ((input % 2) == 0) {
            counter = 0;
            return false;
        }

        if (input != lastOddValue) {
            lastOddValue = input;
            counter = 1;
        } else if (counter < delayBeforeGoingHigh) {
            ++counter;
        }
        return counter >= delayBeforeGoingHigh;
    }

    float process(float input) {
        if (oddTracker(static_cast<int>(input) / stepSize)) {
            return static_cast<float>(1.f);
        }
        return 0.f;
    }
};

struct Btfld : Module {
	enum ParamId {
		GAIN_PARAM,
		CV_PARAM,
		RANGE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_INPUT,
		CV_INPUT,
		INJECT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SAW_OUTPUT,
        ENUMS(BIT_OUTPUT, NIBBLE),
        STEP_OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
        ENUMS(LEVEL_LIGHT, 8),
		ENUMS(SAW_INDICATOR_LIGHT, 3),
		ENUMS(INPUT_INDICATOR_LIGHT, 3),
		ENUMS(CV_INDICATOR_LIGHT, 3),
        ENUMS(INJECT_INDICATOR_LIGHT, 3),
        ENUMS(BIT_INDICATOR_LIGHT, NIBBLE),
        STEP_INDICATOR_LIGHT,
		LIGHTS_LEN
	};

    float feedback;
    float previousInputSignal;
    int previousSteps;
    std::array<float, NIBBLE> bits;
    std::array<float, NIBBLE> bitFilter;
    const int OVERSAMPLING = 512;

    ACCouplingFilter stepFilter;
    ACCouplingFilter sawFilter;

    dsp::Upsampler<BTFLD_UPSAMPLE_RATE, BTFLD_UPSAMPLE_QUALITY> inputUpsampler{0.5f};
    dsp::Upsampler<BTFLD_UPSAMPLE_RATE, BTFLD_UPSAMPLE_QUALITY> cvUpsampler{0.5f};
    dsp::Upsampler<BTFLD_UPSAMPLE_RATE, BTFLD_UPSAMPLE_QUALITY> injectUpsampler{0.5f};

    std::array<float, BTFLD_UPSAMPLE_RATE> upsampledInput;
    std::array<float, BTFLD_UPSAMPLE_RATE> upsampledCV;
    std::array<float, BTFLD_UPSAMPLE_RATE> upsampledInject;
    std::array<float, BTFLD_UPSAMPLE_RATE> workingBuffer;
    std::array<float, BTFLD_UPSAMPLE_RATE> upsampledStepOut;
    std::array<float, BTFLD_UPSAMPLE_RATE> upsampledSaw;

    std::array<dsp::Decimator<BTFLD_UPSAMPLE_RATE, BTFLD_UPSAMPLE_QUALITY>, 4> downsamplers;
    dsp::Decimator<BTFLD_UPSAMPLE_RATE, BTFLD_UPSAMPLE_QUALITY> stepDownsampler;
    dsp::Decimator<BTFLD_UPSAMPLE_RATE, BTFLD_UPSAMPLE_QUALITY> sawDownsampler;

    std::array<BitCalculator, NIBBLE> bitCalculators;

    float upsamplerGain, downsamplerGain;

	Btfld() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(GAIN_PARAM, 0.f, 2.f, 1.f, "Gain");
		configParam(CV_PARAM, 0.f, 1.f, 0.f, "Gain CV");
		configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "Range (+10/±5)");
		configInput(INPUT_INPUT, "In");
		configInput(CV_INPUT, "CV");
		configInput(INJECT_INPUT, "Inject");
		configOutput(SAW_OUTPUT, "Saw");
		configOutput(BIT_OUTPUT + 3, "Out 8");
		configOutput(BIT_OUTPUT + 2, "Out 4");
		configOutput(BIT_OUTPUT + 1, "Out 2");
        configOutput(BIT_OUTPUT, "Out 1");
        configOutput(STEP_OUT_OUTPUT, "Step");

        std::fill(workingBuffer.begin(), workingBuffer.end(), 0.f);

        feedback = 0; previousSteps = 0; previousInputSignal = 0;


        float kernelSum = 0;
        for (auto i = 0; i < BTFLD_UPSAMPLE_RATE * BTFLD_UPSAMPLE_QUALITY; ++i) {
            kernelSum += cvUpsampler.kernel[i];
        }
        upsamplerGain = 1.f / kernelSum;
        kernelSum = 0;
        for (auto i = 0; i < BTFLD_UPSAMPLE_RATE * BTFLD_UPSAMPLE_QUALITY; ++i) {
            kernelSum += downsamplers[0].kernel[i];
        }
        downsamplerGain = 1.f / kernelSum;

        bitCalculators[0].stepSize = 1;
        bitCalculators[1].stepSize = 2;
        bitCalculators[2].stepSize = 4;
        bitCalculators[3].stepSize = 8;
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        stepFilter.setDecay(0.25f * e.sampleRate);
        sawFilter.setDecay(0.25f * e.sampleRate);

        for (auto& b : bits) { b = 0; }
        for (auto& b : bitFilter) { b = 0; }
    }

    float saturate(float x) {
        return std::min(std::max(0.f, x), 11.7f);
    }

    void setPosNegLight(int light, float voltage, float sampleTime) {
        // red is negative, blue is positive
        lights[light + 0].setBrightnessSmooth(std::min(std::max(0.f, -voltage), 5.f) * 0.2f, sampleTime);
        lights[light + 2].setBrightnessSmooth(std::min(std::max(0.f, voltage), 5.f) * 0.2f, sampleTime);
    }

    void process(const ProcessArgs& args) override {
        auto cvInput = inputs[CV_INPUT].isConnected() ? inputs[CV_INPUT].getVoltage() : feedback;
        auto gain = params[GAIN_PARAM].getValue() + params[CV_PARAM].getValue() * cvInput * 0.1f;

        cvUpsampler.process(gain * upsamplerGain, upsampledCV.data());

        setPosNegLight(CV_INDICATOR_LIGHT, params[CV_PARAM].getValue() * cvInput, args.sampleTime);


        auto inputSignal = inputs[INPUT_INPUT].getVoltage();
        auto bipolar = params[RANGE_PARAM].getValue() > 0.5f;
        setPosNegLight(INPUT_INDICATOR_LIGHT, inputSignal, args.sampleTime);


        inputUpsampler.process(inputSignal * upsamplerGain, upsampledInput.data());

        auto inject = inputs[INJECT_INPUT].getVoltage();
        setPosNegLight(INJECT_INDICATOR_LIGHT, inject, args.sampleTime);

        injectUpsampler.process(inject * upsamplerGain, upsampledInject.data());

        for (auto ss = 0; ss < BTFLD_UPSAMPLE_RATE; ++ss) {
            upsampledInput[ss] *= upsampledCV[ss];
            upsampledInput[ss] += bipolar ? 5.f : 0.f;
            upsampledInput[ss] += upsampledInject[ss];

            upsampledInput[ss] = saturate(upsampledInput[ss]);

            upsampledInput[ss] *= (16.f / 10.f);


            upsampledStepOut[ss] = std::max(upsampledInput[ss] - 15.99f, 0.f);
            upsampledInput[ss] = std::min(upsampledInput[ss], 15.99f);

            upsampledStepOut[ss] += std::floor(upsampledInput[ss]);
            upsampledSaw[ss] = std::min(std::max(0.f, upsampledInput[ss] - upsampledStepOut[ss]), 1.1f);
        }



        for (auto b = 0; b < NIBBLE; ++b) {
            for (int ss = 0; ss < BTFLD_UPSAMPLE_RATE; ++ss) {
                workingBuffer[ss] = bitCalculators[b].process(upsampledInput[ss]);
            }
            bits[b] = downsamplers[b].process(workingBuffer.data()) * downsamplerGain;
        }

        for (auto i = 0; i < NIBBLE; ++i) {
            outputs[BIT_OUTPUT + i].setVoltage(bits[i] * 10.f - (bipolar ? 5.f : 0.f));
            lights[BIT_INDICATOR_LIGHT + i].setBrightnessSmooth(bits[i], args.sampleTime);
        }

        float steps = stepDownsampler.process(upsampledStepOut.data()) * downsamplerGain;
        float saw = sawDownsampler.process(upsampledSaw.data()) * downsamplerGain;

        for (int l = 0; l < 8; ++l) {
            // each light covers 2 steps
            auto brightness = 0.f;
            if (bipolar && l < 4) {
                // if bipolar, invert the lower half of the lights to have a "measuring from the midpoint" effect
                brightness += l * 2 > steps ? 0.5f : 0.f;
                brightness += l * 2 + 1 > steps ? 0.5f : 0.f;
            } else {
                brightness += l * 2 <= steps ? 0.5f : 0.f;
                brightness += l * 2 + 1 <= steps ? 0.5f : 0.f;
            }
            lights[LEVEL_LIGHT + l].setBrightnessSmooth(brightness, args.sampleTime);
        }


        saw *= 10.f;
        auto rescaledSteps = steps * (10.f / 16.f);
        auto filteredSteps = stepFilter.process(rescaledSteps);
        outputs[STEP_OUT_OUTPUT].setVoltage(bipolar ? filteredSteps : rescaledSteps);
        auto filteredSaw = sawFilter.process(saw);
        feedback = std::min(std::max(-12.f, (bipolar ? filteredSaw : saw)), 12.f);

        previousInputSignal = inputSignal;
        previousSteps = steps;
        setPosNegLight(SAW_INDICATOR_LIGHT, feedback, args.sampleTime);
        outputs[SAW_OUTPUT].setVoltage(feedback);
//        outputs[SAW_OUTPUT].setVoltage(saw);
    }
};


struct BtfldWidget : ModuleWidget {
	BtfldWidget(Btfld* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/btfld.svg"),
                             asset::plugin(pluginInstance, "res/btfld-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<SchlappiSilverKnob>(mm2px(Vec(15.22, 16.406)), module, Btfld::GAIN_PARAM));
		addParam(createParamCentered<SchlappiSilverKnob>(mm2px(Vec(15.22, 41.785)), module, Btfld::CV_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(7.217, 58.613)), module, Btfld::RANGE_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.196, 72.886)), module, Btfld::INPUT_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.196, 85.892)), module, Btfld::CV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.196, 98.729)), module, Btfld::INJECT_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(20.541, 60.048)), module, Btfld::SAW_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(20.541, 72.886)), module, Btfld::BIT_OUTPUT + 3));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(20.541, 85.892)), module, Btfld::BIT_OUTPUT + 2));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(20.539, 98.809)), module, Btfld::BIT_OUTPUT + 1));
        addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(20.541, 111.736)), module, Btfld::BIT_OUTPUT + 0));
        addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.196, 111.736)), module, Btfld::STEP_OUT_OUTPUT));

		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 11.655)), module, Btfld::LEVEL_LIGHT + 7));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 16.659)), module, Btfld::LEVEL_LIGHT + 6));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 21.663)), module, Btfld::LEVEL_LIGHT + 5));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 26.646)), module, Btfld::LEVEL_LIGHT + 4));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 31.629)), module, Btfld::LEVEL_LIGHT + 3));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 36.654)), module, Btfld::LEVEL_LIGHT + 2));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 41.637)), module, Btfld::LEVEL_LIGHT + 1));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 46.62)), module, Btfld::LEVEL_LIGHT + 0));
		addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(27.213, 53.376)), module, Btfld::SAW_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(13.868, 66.552)), module, Btfld::INPUT_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 66.552)), module, Btfld::BIT_INDICATOR_LIGHT + 3));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 79.389)), module, Btfld::BIT_INDICATOR_LIGHT + 2));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 92.543)), module, Btfld::BIT_INDICATOR_LIGHT + 1));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 105.232)), module, Btfld::BIT_INDICATOR_LIGHT + 0));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(13.868, 79.389)), module, Btfld::CV_INDICATOR_LIGHT));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(13.868, 92.543)), module, Btfld::INJECT_INDICATOR_LIGHT));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(13.868, 105.232)), module, Btfld::STEP_INDICATOR_LIGHT));
	}
};


Model* modelBtfld = createModel<Btfld, BtfldWidget>("btfld");