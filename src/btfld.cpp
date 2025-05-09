#include "plugin.hpp"
#include "widgets/schlappi_widgets.hpp"
#include <iostream>
#include <cmath>
#include <array>
#include <vector>

#define NIBBLE 4

struct DelayedRiser {
    // This emulates something that we observed in the oscilloscope: after the input went high, the output would be
    // delayed by about 15 khz, but there would be no delay when the input goes low. This way very short blips don't
    // register. Implementing this should help with aliasing
    DelayedRiser() {
        setDelay(0);
    }

    void setDelay(int _delay) {
        numSamplesHigh = 0;
        delay = _delay;
    }

    bool process(bool input) {
        bool result = input;
#if true
        if ((input) && (numSamplesHigh >= delay)) {
            result = true;
        } else if (input) {
            result = false;
            numSamplesHigh++;
        } else {
            result = false;
            numSamplesHigh = 0;
        }
#endif
        return result;
    }

    int delay;
    int numSamplesHigh;
};

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

float saturate(float x) {
    if ((-9 <= x) && (x <= 9)) {
        return x;
    } else if (x < -9) {
        return std::tanh(x + 9) - 9;
    } else {
        return std::tanh(x - 9) + 9;
    }
}

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

    const int OVERSAMPLING = 128;

    ACCouplingFilter stepFilter;
    ACCouplingFilter sawFilter;
    std::array<DelayedRiser, NIBBLE> delayedRisers;

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

        feedback = 0; previousSteps = 0; previousInputSignal = 0;
	}

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        stepFilter.setDecay(0.25f * e.sampleRate);
        sawFilter.setDecay(0.25f * e.sampleRate);
        for (auto& d : delayedRisers) {
            d.setDelay(std::round(e.sampleRate * OVERSAMPLING / 20000.f));
        }
        for (auto& b : bits) { b = 0; }
    }

    void calculateInterpolatedBits(float first, float second) {
#if 1
        auto increment = (second - first) / static_cast<float>(OVERSAMPLING);
        // for (auto& b : bits) { b = 0; }
        auto div = 1.f / OVERSAMPLING;
        for (auto s = 0; s < OVERSAMPLING; ++s) {
            auto subsample = static_cast<int>(std::floor(first + increment * s));
            for (auto b = 0; b < NIBBLE; ++b) {
                //bits[b] += delayedRisers[b].process(subsample & (1 << b)) ? div : 0;
                const float filterTerm = 0.99f;
                bits[b] = bits[b] * filterTerm + (delayedRisers[b].process(subsample & (1 << b)) ? (1.f - filterTerm) : 0.f);
            }
        }
#else
        auto lower = std::min(first, second);
        auto higher = std::max(first, second);
        if (std::floor(higher) - std::floor(lower) < 0.01) {
            for (auto i = 0; i < 4; ++i) {
                bits[i] = (static_cast<int>(std::floor(lower)) & (1 << i)) ? 1.f : 0.f;
            }
            return;
        }
        for (auto& b : bits) { b = 0; }
        auto totalWeightScalar = 1 / (higher - lower);
        for (int step = std::floor(lower); step <= std::floor(higher - 0.001); ++step) {
            auto weight = 1.f;
            if (step == std::floor(lower)) {
                weight -= lower - std::floor(lower);
            }
            if (step == std::floor(higher)) {
                weight -= std::ceil(higher) - higher;
            }
            for (auto i = 0; i < 4; ++i) {
                bits[i] += ((step & (1 << i)) ? 1.f : 0.f) * weight * totalWeightScalar;
            }
        }
#endif
    }

    void setPosNegLight(int light, float voltage, float sampleTime) {
        // red is negative, blue is positive
        lights[light + 0].setBrightnessSmooth(std::min(std::max(0.f, -voltage), 5.f) * 0.2f, sampleTime);
        lights[light + 2].setBrightnessSmooth(std::min(std::max(0.f, voltage), 5.f) * 0.2f, sampleTime);
    }

    void process(const ProcessArgs& args) override {
        auto cvInput = inputs[CV_INPUT].isConnected() ? inputs[CV_INPUT].getVoltage() : feedback;
        auto gain = params[GAIN_PARAM].getValue() + params[CV_PARAM].getValue() * cvInput * 0.1f;

        setPosNegLight(CV_INDICATOR_LIGHT, params[CV_PARAM].getValue() * cvInput, args.sampleTime);

        auto inputSignal = inputs[INPUT_INPUT].getVoltage();
        auto bipolar = params[RANGE_PARAM].getValue() > 0.5f;
        setPosNegLight(INPUT_INDICATOR_LIGHT, inputSignal, args.sampleTime);
        inputSignal *= gain;
        inputSignal += bipolar ? 5.f : 0.f;

        auto inject = inputs[INJECT_INPUT].getVoltage();
        setPosNegLight(INJECT_INDICATOR_LIGHT, inject, args.sampleTime);

        inputSignal += inject;

        inputSignal = std::max(inputSignal, 0.f);
        inputSignal = std::min(inputSignal, 11.7f); // TODO: more natural saturation
        inputSignal *= (15.f / 10.f);
        auto steps = static_cast<int>(floor(inputSignal));
        steps = std::max(0, std::min(steps, 15));

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

        calculateInterpolatedBits(previousInputSignal, previousInputSignal);
        for (auto i = 0; i < NIBBLE; ++i) {
            outputs[BIT_OUTPUT + i].setVoltage(bits[i] * 10.f - (bipolar ? 5.f : 0.f));
            lights[BIT_INDICATOR_LIGHT + i].setBrightnessSmooth(bits[i], args.sampleTime);
        }

        auto rescaledSteps = static_cast<float>(steps) * (10.f / 15.f);
        auto filteredSteps = stepFilter.process(rescaledSteps);
        outputs[STEP_OUT_OUTPUT].setVoltage(saturate(bipolar ? filteredSteps : rescaledSteps));
        auto saw = inputSignal - rescaledSteps;
        auto filteredSaw = sawFilter.process(saw);
        feedback = saturate((bipolar ? filteredSaw : saw) * 10.f);

        previousInputSignal = inputSignal;
        previousSteps = steps;
        setPosNegLight(SAW_INDICATOR_LIGHT, feedback, args.sampleTime);
        outputs[SAW_OUTPUT].setVoltage(feedback);
    }
};


struct BtfldWidget : ModuleWidget {
	BtfldWidget(Btfld* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/btfld.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<SchlappiSilverKnob>(mm2px(Vec(15.22, 16.406)), module, Btfld::GAIN_PARAM));
		addParam(createParamCentered<SchlappiSilverKnob>(mm2px(Vec(15.22, 41.785)), module, Btfld::CV_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(7.217, 58.613)), module, Btfld::RANGE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.196, 72.886)), module, Btfld::INPUT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.196, 85.892)), module, Btfld::CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.196, 98.729)), module, Btfld::INJECT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 60.048)), module, Btfld::SAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 72.886)), module, Btfld::BIT_OUTPUT + 3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 85.892)), module, Btfld::BIT_OUTPUT + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.539, 98.809)), module, Btfld::BIT_OUTPUT + 1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 111.736)), module, Btfld::BIT_OUTPUT + 0));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.196, 111.736)), module, Btfld::STEP_OUT_OUTPUT));

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