#include "plugin.hpp"
#include "widgets/schlappi_widgets.hpp"
#include <array>


#define NIBBLER_UPSAMPLE_RATIO 16
#define NIBBLER_UPSAMPLE_QUALITY 4
#define NIBBLER_NUM_BITS 4


struct UpsampledTrigger {
    UpsampledTrigger() : upsampler(0.7f) {}
    std::array<float, NIBBLER_UPSAMPLE_RATIO> input;
    dsp::Upsampler<NIBBLER_UPSAMPLE_RATIO, NIBBLER_UPSAMPLE_QUALITY> upsampler;
    dsp::SchmittTrigger trigger;

    void process(float in) {
        upsampler.process(in, input.data());
    }
};

struct NibbleRegister {
    unsigned char heldValue;
    NibbleRegister() : heldValue(0) {}
    unsigned char process(unsigned char input, bool shift, bool shiftData, bool clock, bool reset) {
        if (clock) {
            heldValue = input & 15;
            heldValue <<= shift ? 1 : 0;
            heldValue += shift && shiftData ? 1 : 0;
        }
        if (reset) {
            heldValue = 0;
        }
        return heldValue;
    }
};

struct Nibbler : Module {
	enum ParamId {
		ADD_8_PARAM,
		OFFSET_1_PARAM,
		RESET_PARAM,
		ADD_4_PARAM,
		OFFSET_2_PARAM,
		SUBTRACT_ADD_PARAM,
		ASYNC_SYNC_PARAM,
		ADD_2_PARAM,
		ADD_1_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CARRY_IN_INPUT,
		GATE_8_INPUT,
		CLOCK_INPUT,
		SHIFT_INPUT,
		GATE_4_INPUT,
		RESET_INPUT,
		SHIFT_DATA_INPUT,
		GATE_2_INPUT,
		SUB_INPUT,
		DATA_XOR_INPUT,
		GATE_1_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OFFSET_STEP_OUTPUT,
		STEP_OUTPUT,
		CARRY_OUTPUT,
		OUT_8_OUTPUT,
		OUT_4_OUTPUT,
		OUT_2_OUTPUT,
		OUT_1_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		OFFSET_STEP_LIGHT,
		STEP_LIGHT,
		CARRY_LIGHT,
		CARRY_IN_LIGHT,
		GATE_8_LIGHT,
		OUT_8_LIGHT,
		CLOCK_LIGHT,
		SHIFT_LIGHT,
		GATE_4_LIGHT,
		OUT_4_LIGHT,
		RESET_LIGHT,
		OUT_2_LIGHT,
		SHIFT_DATA_LIGHT,
		GATE_2_LIGHT,
		SUB_LIGHT,
		DATA_XOR_LIGHT,
		GATE_1_LIGHT,
		OUT_1_LIGHT,
		LIGHTS_LEN
	};

    std::array<dsp::Decimator<NIBBLER_UPSAMPLE_RATIO, NIBBLER_UPSAMPLE_QUALITY>, NIBBLER_NUM_BITS + 1> bitOutDecimators;

    std::array<unsigned char, NIBBLER_UPSAMPLE_RATIO> inputBytes;

    std::array<UpsampledTrigger, NIBBLER_UPSAMPLE_RATIO> gateUTrig;

    std::array<std::array<float, NIBBLER_UPSAMPLE_RATIO>, NIBBLER_NUM_BITS + 1> upsampledBitOutput;

    UpsampledTrigger carryInUTrig;
    UpsampledTrigger subtractUTrig;
    UpsampledTrigger resetUTrig;
    UpsampledTrigger clockUTrig;
    UpsampledTrigger shiftUTrig;
    UpsampledTrigger shiftDataUTrig;
    UpsampledTrigger shiftXorUTrig;

    std::array<unsigned char, NIBBLER_UPSAMPLE_RATIO> accumulatorOutBytes;

    dsp::Decimator<NIBBLER_UPSAMPLE_RATIO, NIBBLER_UPSAMPLE_QUALITY> stepDecimator;
    dsp::Decimator<NIBBLER_UPSAMPLE_RATIO, NIBBLER_UPSAMPLE_QUALITY> offsetStepDecimator;
    std::array<float, NIBBLER_UPSAMPLE_RATIO> stepDecimatorInput;
    std::array<float, NIBBLER_UPSAMPLE_RATIO> offsetStepDecimatorInput;

    const std::array<InputId, NIBBLER_NUM_BITS> gateInputIds {
        GATE_1_INPUT, GATE_2_INPUT, GATE_4_INPUT, GATE_8_INPUT
    };
    const std::array<LightId, NIBBLER_NUM_BITS> gateLightIds {
        GATE_1_LIGHT, GATE_2_LIGHT, GATE_4_LIGHT, GATE_8_LIGHT
    };
    const std::array<ParamId, NIBBLER_NUM_BITS> addParamIds {
        ADD_1_PARAM, ADD_2_PARAM, ADD_4_PARAM, ADD_8_PARAM
    };
    const std::array<OutputId, NIBBLER_NUM_BITS + 1> outputBitIds {
        OUT_1_OUTPUT, OUT_2_OUTPUT, OUT_4_OUTPUT, OUT_8_OUTPUT, CARRY_OUTPUT
    };
    const std::array<LightId, NIBBLER_NUM_BITS + 1> outputLightIds {
        OUT_1_LIGHT, OUT_2_LIGHT, OUT_4_LIGHT, OUT_8_LIGHT, CARRY_LIGHT
    };

    float out8;
    float gateVoltage;

    NibbleRegister nibbleRegister;

	Nibbler() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(ADD_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OFFSET_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ADD_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OFFSET_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SUBTRACT_ADD_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ASYNC_SYNC_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ADD_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ADD_1_PARAM, 0.f, 1.f, 0.f, "");
		configInput(CARRY_IN_INPUT, "");
		configInput(GATE_8_INPUT, "");
		configInput(CLOCK_INPUT, "");
		configInput(SHIFT_INPUT, "");
		configInput(GATE_4_INPUT, "");
		configInput(RESET_INPUT, "");
		configInput(SHIFT_DATA_INPUT, "");
		configInput(GATE_2_INPUT, "");
		configInput(SUB_INPUT, "");
		configInput(DATA_XOR_INPUT, "");
		configInput(GATE_1_INPUT, "");
		configOutput(OFFSET_STEP_OUTPUT, "");
		configOutput(STEP_OUTPUT, "");
		configOutput(CARRY_OUTPUT, "");
		configOutput(OUT_8_OUTPUT, "");
		configOutput(OUT_4_OUTPUT, "");
		configOutput(OUT_2_OUTPUT, "");
		configOutput(OUT_1_OUTPUT, "");

        out8 = 0;

        // std::fill(gateUpsamplers.begin(), gateUpsamplers.end(), 0.4f);
        std::fill(bitOutDecimators.begin(), bitOutDecimators.end(), 0.8f);
        std::fill(accumulatorOutBytes.begin(), accumulatorOutBytes.end(), 0);

        // the convolution kernel in the vcvrack upsampler/decimator does not sum to 1, so we have to compensate that
        // when generating upsampled pulses, so that they will downsample to 10 volts.
        float kernelSum = 0;
        for (auto i = 0; i < NIBBLER_UPSAMPLE_RATIO * NIBBLER_UPSAMPLE_QUALITY; ++i) {
            kernelSum += bitOutDecimators[0].kernel[i];
        }
        gateVoltage = 10.f / kernelSum;
    }

    void computeInputBytes(const ProcessArgs& args) {
        for (auto& b : inputBytes) { b = 0; }

        for (auto b = 0; b < NIBBLER_NUM_BITS; ++b) {
            gateUTrig[b].process(inputs[gateInputIds[b]].getVoltage());

            for (auto s = 0; s < NIBBLER_UPSAMPLE_RATIO; ++s) {
                gateUTrig[b].trigger.process(gateUTrig[b].input[s], 0.1f, 1.0f);
                inputBytes[s] += (gateUTrig[b].trigger.isHigh() ? 1 : 0) << b;
            }

            lights[gateLightIds[b]].setBrightnessSmooth(gateUTrig[b].trigger.isHigh(), args.sampleTime);
        }

        carryInUTrig.process(inputs[CARRY_IN_INPUT].getVoltage());

        for (auto s = 0; s < NIBBLER_UPSAMPLE_RATIO; ++s) {
            carryInUTrig.trigger.process(carryInUTrig.input[s], 0.1f, 1.0f);
            inputBytes[s] += carryInUTrig.trigger.isHigh() ? 1 : 0;
        }

        lights[CARRY_IN_LIGHT].setBrightnessSmooth(carryInUTrig.trigger.isHigh(), args.sampleTime);

        unsigned char add = 0;

        add += (params[ADD_1_PARAM].getValue() > 0.5f) ? 1 : 0;
        add += (params[ADD_2_PARAM].getValue() > 0.5f) ? 2 : 0;
        add += (params[ADD_4_PARAM].getValue() > 0.5f) ? 4 : 0;
        add += (params[ADD_8_PARAM].getValue() > 0.5f) ? 8 : 0;

        for (auto& s : inputBytes) {
            s += add;
        }

        subtractUTrig.process(inputs[SUB_INPUT].getVoltage());
        bool subtractSwitch = (params[SUBTRACT_ADD_PARAM].getValue() > 0.5f);
        for (auto s = 0; s < NIBBLER_UPSAMPLE_RATIO; ++s) {
            subtractUTrig.trigger.process(subtractUTrig.input[s], 0.1, 1.f);
            if (subtractSwitch != (subtractUTrig.trigger.isHigh())) {
                inputBytes[s] = 16 - (inputBytes[s] & 15);
            }
        }

        lights[SUB_LIGHT].setBrightnessSmooth((subtractSwitch != subtractUTrig.trigger.isHigh()) ? 1.f : 0.f, args.sampleTime);
    }

	void process(const ProcessArgs& args) override {
        computeInputBytes(args);

        /* Set accumulator parameters */
        resetUTrig.process(inputs[RESET_INPUT].getVoltage());
        clockUTrig.process(inputs[CLOCK_INPUT].getVoltage());
        shiftUTrig.process(inputs[SHIFT_INPUT].getVoltage());
        shiftDataUTrig.process(
                inputs[SHIFT_DATA_INPUT].isConnected()
                ? inputs[SHIFT_DATA_INPUT].getVoltage()
                : out8);
        shiftXorUTrig.process(inputs[DATA_XOR_INPUT].getVoltage());

        bool resetButtonDown = params[RESET_PARAM].getValue() > 0.5f;
        /* reset light is only based on the button, not the jack input */
        lights[RESET_LIGHT].setBrightnessSmooth(resetButtonDown, args.sampleTime);

        bool async = (params[ASYNC_SYNC_PARAM].getValue() > 0.5f) || !inputs[CLOCK_INPUT].isConnected();

        for (auto s = 0; s < NIBBLER_UPSAMPLE_RATIO; ++s) {
            inputBytes[s] += nibbleRegister.heldValue;
            shiftDataUTrig.trigger.process(shiftDataUTrig.input[s]);
            shiftXorUTrig.trigger.process(shiftXorUTrig.input[s]);

            auto hiShift = shiftUTrig.trigger.process(shiftUTrig.input[s], 0.1f, 1.f);
            auto hiClock = clockUTrig.trigger.process(clockUTrig.input[s], 0.1f, 1.f);

            hiClock = async ? (hiClock || hiShift) : hiClock;

            auto s1 = inputs[SHIFT_DATA_INPUT].isConnected() ? shiftDataUTrig.trigger.isHigh() : out8;
            auto s2 = shiftXorUTrig.trigger.isHigh();

            auto shiftDataInput = (s1 != s2);

            resetUTrig.trigger.process(resetUTrig.input[s], 0.1f, 1.f);

            nibbleRegister.process(inputBytes[s],
                                   shiftUTrig.trigger.isHigh(),
                                   shiftDataInput,
                                   hiClock,
                                   (resetUTrig.trigger.isHigh() || resetButtonDown));
            if (async) {
                accumulatorOutBytes[s] = inputBytes[s];
            } else {
                // carry always comes from the summed input bytes, it is not held in the register
                accumulatorOutBytes[s] = nibbleRegister.heldValue | (inputBytes[s] & 16);
            }
        }

        lights[CLOCK_LIGHT].setBrightnessSmooth(clockUTrig.trigger.isHigh(), args.sampleTime);
        lights[SHIFT_LIGHT].setBrightnessSmooth(shiftUTrig.trigger.isHigh(), args.sampleTime);
        lights[SHIFT_DATA_LIGHT].setBrightnessSmooth(inputs[SHIFT_DATA_INPUT].isConnected() ? shiftDataUTrig.trigger.isHigh() : out8, args.sampleTime);
        lights[DATA_XOR_LIGHT].setBrightnessSmooth(shiftXorUTrig.trigger.isHigh(), args.sampleTime);




        for (auto b = 0; b < NIBBLER_NUM_BITS + 1; ++b) {
            for (auto s = 0; s < NIBBLER_UPSAMPLE_RATIO; ++s) {
                auto outByte = async ? inputBytes[s] : accumulatorOutBytes[s];
                upsampledBitOutput[b][s] = (outByte & (1 << b)) ? gateVoltage : 0.f;
            }
            auto outVolt = bitOutDecimators[b].process(upsampledBitOutput[b].data());
            lights[outputLightIds[b]].setBrightnessSmooth(outVolt * 0.1f, args.sampleTime);
            outputs[outputBitIds[b]].setVoltage(outVolt);
            if (b == 3) {
                out8 = outVolt;
            }
        }

        std::fill(stepDecimatorInput.begin(), stepDecimatorInput.end(), 0);
        std::fill(offsetStepDecimatorInput.begin(), offsetStepDecimatorInput.end(), 0);

        auto s1 = params[OFFSET_1_PARAM].getValue() > 0.5f;
        auto s2 = params[OFFSET_2_PARAM].getValue() > 0.5f;

        unsigned char stepOffset = 0;

        if (s1 && !s2) {
            stepOffset = 4;
        } else if (!s1 && s2) {
            stepOffset = 2;
        } else if (s1 && s2) {
            stepOffset = 8;
        }

        for (auto s = 0; s < NIBBLER_UPSAMPLE_RATIO; ++s) {
            auto outByte = async ? inputBytes[s] : accumulatorOutBytes[s];
            stepDecimatorInput[s] = static_cast<float>(outByte & 15) * (gateVoltage / 16.f);
            offsetStepDecimatorInput[s] = static_cast<float>((outByte + stepOffset) & 15) * (gateVoltage / 16.f);
        }

        auto stepOut = stepDecimator.process(stepDecimatorInput.data());
        outputs[STEP_OUTPUT].setVoltage(stepOut);
        lights[STEP_LIGHT].setBrightnessSmooth(stepOut * 0.1f, args.sampleTime);

        auto offsetStepOut = offsetStepDecimator.process(offsetStepDecimatorInput.data());
        outputs[OFFSET_STEP_OUTPUT].setVoltage(offsetStepOut);
        lights[OFFSET_STEP_LIGHT].setBrightnessSmooth(offsetStepOut * 0.1f, args.sampleTime);
    }
};


struct NibblerWidget : ModuleWidget {
	NibblerWidget(Nibbler* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/nibbler.svg"),
                             asset::plugin(pluginInstance, "res/nibbler-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        auto buttonLight = createLightCentered<RectangleLight<BlueLight>>(mm2px(Vec(29.56, 24.661)), module, Nibbler::RESET_LIGHT);
        auto buttonCenter = 19.974 * 0.5;
        auto x = 8.737;
        auto y = 1.261;
        auto pos = mm2px(Vec(x - buttonCenter, y - buttonCenter));
        auto w = 2.521;
        auto h  = 4.825;
        auto size = mm2px(Vec(w,h));
        buttonLight->box.pos = mm2px(Vec(29.56, 24.661)) + pos;
        buttonLight->box.size = size;
        addChild(buttonLight);


        addParam(createParamCentered<SchlappiCherryMXBrown>(mm2px(Vec(29.56, 24.661)), module, Nibbler::RESET_PARAM));
        addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(10.494, 16.384)), module, Nibbler::ADD_8_PARAM));
        addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(49.512, 16.384)), module, Nibbler::OFFSET_1_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(10.494, 31.629)), module, Nibbler::ADD_4_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(49.512, 31.629)), module, Nibbler::OFFSET_2_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(23.5, 41.595)), module, Nibbler::SUBTRACT_ADD_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(36.506, 41.595)), module, Nibbler::ASYNC_SYNC_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(10.494, 46.831)), module, Nibbler::ADD_2_PARAM));
		addParam(createParamCentered<SchlappiToggleVertical2pos>(mm2px(Vec(10.494, 62.075)), module, Nibbler::ADD_1_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(23.5, 72.674)), module, Nibbler::CARRY_IN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(36.506, 72.674)), module, Nibbler::GATE_8_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.515, 85.681)), module, Nibbler::CLOCK_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(23.5, 85.681)), module, Nibbler::SHIFT_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(36.506, 85.681)), module, Nibbler::GATE_4_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.515, 98.687)), module, Nibbler::RESET_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(23.5, 98.687)), module, Nibbler::SHIFT_DATA_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(36.506, 98.687)), module, Nibbler::GATE_2_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.515, 111.693)), module, Nibbler::SUB_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(23.5, 111.693)), module, Nibbler::DATA_XOR_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(36.506, 111.693)), module, Nibbler::GATE_1_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(49.491, 46.535)), module, Nibbler::OFFSET_STEP_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(36.506, 59.668)), module, Nibbler::STEP_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(49.491, 59.668)), module, Nibbler::CARRY_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(49.491, 72.717)), module, Nibbler::OUT_8_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(49.491, 85.681)), module, Nibbler::OUT_4_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(49.491, 98.687)), module, Nibbler::OUT_2_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(49.491, 111.693)), module, Nibbler::OUT_1_OUTPUT));

		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(55.53, 40.666)), module, Nibbler::OFFSET_STEP_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(42.861, 53.503)), module, Nibbler::STEP_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(55.53, 53.503)), module, Nibbler::CARRY_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(29.855, 66.171)), module, Nibbler::CARRY_IN_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(42.861, 66.171)), module, Nibbler::GATE_8_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(55.53, 66.171)), module, Nibbler::OUT_8_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(16.849, 79.178)), module, Nibbler::CLOCK_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(29.855, 79.178)), module, Nibbler::SHIFT_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(42.861, 79.178)), module, Nibbler::GATE_4_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(55.868, 79.178)), module, Nibbler::OUT_4_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(16.849, 92.522)), module, Nibbler::RESET_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(55.868, 92.184)), module, Nibbler::OUT_2_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(29.855, 92.522)), module, Nibbler::SHIFT_DATA_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(42.861, 92.522)), module, Nibbler::GATE_2_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(16.849, 105.19)), module, Nibbler::SUB_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(29.855, 105.19)), module, Nibbler::DATA_XOR_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(42.861, 105.19)), module, Nibbler::GATE_1_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(55.868, 105.19)), module, Nibbler::OUT_1_LIGHT));
	}
};


Model* modelNibbler = createModel<Nibbler, NibblerWidget>("Nibbler");