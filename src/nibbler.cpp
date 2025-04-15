#include "plugin.hpp"

struct InputSwitch {
    InputSwitch(Param& _param, Input& _input) : param(_param), input(_input) {
        inputTrigger.reset();
        mainTrigger.reset();
        trig = false;
    }

    bool process() {
        inputTrigger.process(input.getVoltage(), 0.1f, 1.5f);
        trig = mainTrigger.process(inputTrigger.isHigh() || (param.getValue() > 0.5f));
        return trig;
    }

    bool isHigh() {
        return mainTrigger.isHigh();
    }

    bool wasTriggered() {
        return trig;
    }
private:
    dsp::SchmittTrigger inputTrigger;
    dsp::BooleanTrigger mainTrigger;

    Param& param;
    Input& input;

    bool trig;
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

    std::array<dsp::SchmittTrigger, 4> bitInputTriggers;
    dsp::SchmittTrigger subtractTrigger, clockTrigger, resetTrigger, shiftTrigger, dataXorTrigger;
    // Defined in https://vcvrack.com/manual/VoltageStandards
    const float TRIGGER_LOW_THRESHOLD = 0.1f;
    const float TRIGGER_HIGH_THRESHOLD = 1.5f;
    unsigned char outputRegisters;
    float sampleTime;

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
        outputRegisters = 0;
	}

    unsigned char getStep() {
        bitInputTriggers[0].process(inputs[GATE_1_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);
        bitInputTriggers[1].process(inputs[GATE_2_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);
        bitInputTriggers[2].process(inputs[GATE_4_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);
        bitInputTriggers[3].process(inputs[GATE_8_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);

        unsigned char gateInput =
                (bitInputTriggers[0].isHigh() ? 1 : 0) +
                (bitInputTriggers[1].isHigh() ? 2 : 0) +
                (bitInputTriggers[2].isHigh() ? 4 : 0) +
                (bitInputTriggers[3].isHigh() ? 8 : 0);

        unsigned char switchInput =
                (params[ADD_1_PARAM].getValue() > 0.5 ? 1 : 0) +
                (params[ADD_2_PARAM].getValue() > 0.5 ? 2 : 0) +
                (params[ADD_4_PARAM].getValue() > 0.5 ? 4 : 0) +
                (params[ADD_8_PARAM].getValue() > 0.5 ? 8 : 0);

        unsigned char step = (gateInput + switchInput) & 15;

        lights[GATE_1_LIGHT].setBrightnessSmooth(step & 1 ? 1.f : 0.f, sampleTime);
        lights[GATE_2_LIGHT].setBrightnessSmooth(step & 2 ? 1.f : 0.f, sampleTime);
        lights[GATE_4_LIGHT].setBrightnessSmooth(step & 4 ? 1.f : 0.f, sampleTime);
        lights[GATE_8_LIGHT].setBrightnessSmooth(step & 8 ? 1.f : 0.f, sampleTime);

        subtractTrigger.process(inputs[SUB_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);
        bool subtract = subtractTrigger.isHigh() != (params[SUBTRACT_ADD_PARAM].getValue() > 0.5); // logical xor

        lights[SUB_LIGHT].setBrightnessSmooth(subtract ? 1.f : 0.f, sampleTime);

        if (subtract) {
            step = 16 - step;
        }
        return step;
    }

    void setRegisterOutputs(unsigned char nibble) {
        outputs[OUT_1_OUTPUT].setVoltage(nibble & 1 ? 10.f : 0.f);
        outputs[OUT_2_OUTPUT].setVoltage(nibble & 2 ? 10.f : 0.f);
        outputs[OUT_4_OUTPUT].setVoltage(nibble & 4 ? 10.f : 0.f);
        outputs[OUT_8_OUTPUT].setVoltage(nibble & 8 ? 10.f : 0.f);
        outputs[CARRY_OUTPUT].setVoltage(nibble & 16 ? 10.f : 0.f);

        lights[OUT_1_LIGHT].setBrightnessSmooth(nibble & 1 ? 1.f : 0.f, sampleTime);
        lights[OUT_2_LIGHT].setBrightnessSmooth(nibble & 2 ? 1.f : 0.f, sampleTime);
        lights[OUT_4_LIGHT].setBrightnessSmooth(nibble & 4 ? 1.f : 0.f, sampleTime);
        lights[OUT_8_LIGHT].setBrightnessSmooth(nibble & 8 ? 1.f : 0.f, sampleTime);
        lights[CARRY_LIGHT].setBrightnessSmooth(nibble & 16 ? 1.f : 0.f, sampleTime);

    }

    unsigned char getOffset() {
        auto offsetIndex = (params[OFFSET_1_PARAM].getValue() > 0.5f ? 2 : 0)
                           + (params[OFFSET_2_PARAM].getValue() > 0.5 ? 1 : 0);
        switch (offsetIndex) {
            case 0: return 0;
            case 1: return 2;
            case 2: return 4;
            case 4: return 8;
            default: return 0;
        }
    }

    bool getReset() {
        resetTrigger.process(inputs[RESET_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);
        bool reset = resetTrigger.isHigh() || (params[RESET_PARAM].getValue() > 0.5f);
        lights[RESET_LIGHT].setBrightnessSmooth(reset ? 1.f : 0.f, sampleTime);
        return reset;
    }

    void setSteppedOutput(unsigned char resultRegister) {
        auto offset = getOffset();

        auto steppedOut = static_cast<float>(resultRegister & 15) * (10.f / 15.f);
        outputs[STEP_OUTPUT].setVoltage(steppedOut);
        lights[STEP_LIGHT].setBrightnessSmooth(static_cast<float>(resultRegister & 15) / 15.f, sampleTime);
        outputs[OFFSET_STEP_OUTPUT].setVoltage(static_cast<float>((resultRegister + offset) & 15) * 10.f / 15.f);
        lights[OFFSET_STEP_LIGHT].setBrightnessSmooth(static_cast<float>((resultRegister + offset) & 15) / 15.f, sampleTime);
    }

	void process(const ProcessArgs& args) override {
        sampleTime = args.sampleTime;
        auto step = getStep();
        auto reset = getReset();

        bool clockGoingHigh = clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);

        bool shiftGoingHigh = shiftTrigger.process(inputs[SHIFT_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);

        bool isAsync = params[ASYNC_SYNC_PARAM].getValue() > 0.5f;
        unsigned char added = (outputRegisters + step) & 31;

        if ((isAsync && (clockGoingHigh != shiftGoingHigh)) || (!isAsync && clockGoingHigh)) {
            outputRegisters = added;
            if (shiftTrigger.isHigh()) {
                bool shiftInput = inputs[SHIFT_DATA_INPUT].isConnected() ?
                                  (inputs[SHIFT_DATA_INPUT].getVoltage() > TRIGGER_HIGH_THRESHOLD) : (outputRegisters & 8 == 8);

                dataXorTrigger.process(inputs[DATA_XOR_INPUT].getVoltage(), TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);

                shiftInput = (shiftInput != dataXorTrigger.isHigh());

                outputRegisters = (outputRegisters << 1) + shiftInput;
            }
            if (reset) {
                outputRegisters = 0; // TODO: which is first: reset or shift?
            }
        }

        auto resultRegister = isAsync ? added : outputRegisters;

        setRegisterOutputs(resultRegister);
        setSteppedOutput(resultRegister);
	}
};


struct NibblerWidget : ModuleWidget {
	NibblerWidget(Nibbler* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/nibbler.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<CKD6>(mm2px(Vec(30.48, 24.311)), module, Nibbler::RESET_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(10.494, 16.384)), module, Nibbler::ADD_8_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(49.512, 16.384)), module, Nibbler::OFFSET_1_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(10.494, 31.629)), module, Nibbler::ADD_4_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(49.512, 31.629)), module, Nibbler::OFFSET_2_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(23.5, 41.595)), module, Nibbler::SUBTRACT_ADD_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(36.506, 41.595)), module, Nibbler::ASYNC_SYNC_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(10.494, 46.831)), module, Nibbler::ADD_2_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(10.494, 62.075)), module, Nibbler::ADD_1_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.5, 72.674)), module, Nibbler::CARRY_IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.506, 72.674)), module, Nibbler::GATE_8_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.515, 85.681)), module, Nibbler::CLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.5, 85.681)), module, Nibbler::SHIFT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.506, 85.681)), module, Nibbler::GATE_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.515, 98.687)), module, Nibbler::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.5, 98.687)), module, Nibbler::SHIFT_DATA_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.506, 98.687)), module, Nibbler::GATE_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.515, 111.693)), module, Nibbler::SUB_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.5, 111.693)), module, Nibbler::DATA_XOR_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.506, 111.693)), module, Nibbler::GATE_1_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.491, 46.535)), module, Nibbler::OFFSET_STEP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.506, 59.668)), module, Nibbler::STEP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.491, 59.668)), module, Nibbler::CARRY_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.491, 72.717)), module, Nibbler::OUT_8_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.491, 85.681)), module, Nibbler::OUT_4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.491, 98.687)), module, Nibbler::OUT_2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.491, 111.693)), module, Nibbler::OUT_1_OUTPUT));

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