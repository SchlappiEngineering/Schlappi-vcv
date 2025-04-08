#include "plugin.hpp"
#include <rack.hpp>
#include <array>
#include <iostream>

struct BTMX : Module {
	enum ParamId {
        ENUMS(SWITCH_PARAM, 8),
        LOGIC_MODE_A,
        LOGIC_MODE_B,
		PARAMS_LEN
	};
	enum InputId {
        ENUMS(IN_INPUT, 8),
		INPUTS_LEN
	};
	enum OutputId {
		STEP_OUTPUT,
        ENUMS(MIX_OUTPUT, 4),
		OUTPUTS_LEN
	};
	enum LightId {
		STEP_INDICATOR_LIGHT,
        ENUMS(IN_INDICATOR_LIGHT, 8),
        ENUMS(MIX_INDICATOR_LIGHT, 4),
		LIGHTS_LEN
	};

    enum LogicMode {
        AND,
        ADD,
        OR,
        XOR
    };

    std::array<dsp::SchmittTrigger, 8> triggers;

    // Defined in https://vcvrack.com/manual/VoltageStandards
    const float TRIGGER_LOW_THRESHOLD = 0.1f;
    const float TRIGGER_HIGH_THRESHOLD = 1.5f;

    unsigned char inputA, inputB, mix;


    BTMX() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SWITCH_PARAM + 0, 0.f, 1.f, 0.f, "Switch 1");
        configParam(SWITCH_PARAM + 1, 0.f, 1.f, 0.f, "Switch 2");
        configParam(SWITCH_PARAM + 2, 0.f, 1.f, 0.f, "Switch 3");
        configParam(SWITCH_PARAM + 3, 0.f, 1.f, 0.f, "Switch 4");
        configParam(SWITCH_PARAM + 4, 0.f, 1.f, 0.f, "Switch 5");
        configParam(SWITCH_PARAM + 5, 0.f, 1.f, 0.f, "Switch 6");
        configParam(SWITCH_PARAM + 6, 0.f, 1.f, 0.f, "Switch 7");
		configParam(SWITCH_PARAM + 7, 0.f, 1.f, 0.f, "Switch 8");
        configParam(LOGIC_MODE_A, 0.f, 1.f, 0.f, "Logic Mode A");
        configParam(LOGIC_MODE_B, 0.f, 1.f, 0.f, "Logic Mode B");
		configInput(IN_INPUT + 0, "In 1");
		configInput(IN_INPUT + 1, "In 2");
		configInput(IN_INPUT + 2, "In 3");
		configInput(IN_INPUT + 3, "In 4");
		configInput(IN_INPUT + 4, "In 5");
		configInput(IN_INPUT + 5, "In 6");
		configInput(IN_INPUT + 6, "In 7");
		configInput(IN_INPUT + 7, "In 8");
		configOutput(STEP_OUTPUT, "Step");
        configOutput(MIX_OUTPUT + 0, "Mix 1 ★ 5");
        configOutput(MIX_OUTPUT + 1, "Mix 2 ★ 6");
        configOutput(MIX_OUTPUT + 2, "Mix 3 ★ 7");
		configOutput(MIX_OUTPUT + 3, "Mix 4 ★ 8");

        for (auto& trigger : triggers) {
            trigger.reset();
        }
	}

	void process(const ProcessArgs& args) override {
        inputA = '\0';
        inputB = '\0';
        mix = '\0';
        for (int i = 0; i < 8; ++i) {
            auto inputVoltage = params[SWITCH_PARAM + i].getValue() > 0.5 ? inputs[IN_INPUT + i].getVoltage() : 0;
            triggers[i].process(inputVoltage, TRIGGER_LOW_THRESHOLD, TRIGGER_HIGH_THRESHOLD);
            lights[IN_INDICATOR_LIGHT + i].setBrightnessSmooth(triggers[i].isHigh() ? 1.f : 0.f, args.sampleTime);
        }

        inputA = triggers[0].isHigh() * 8 + triggers[1].isHigh() * 4 + triggers[2].isHigh() * 2 + triggers[3].isHigh();
        inputB = triggers[4].isHigh() * 8 + triggers[5].isHigh() * 4 + triggers[6].isHigh() * 2 + triggers[7].isHigh();

        int logicMode =
                (inputs[LOGIC_MODE_A].getVoltage() > 0.5 ? 2 : 0) +
                (inputs[LOGIC_MODE_B].getVoltage() > 0.5 ? 1 : 0);

        std::cout << logicMode << " ";

        switch (logicMode) {
            case 0: mix = inputA & inputB; break;
            case 1: mix = inputA + inputB; break;
            case 2:  mix = inputA | inputB; break;
            case 3: mix = inputA ^ inputB; break;
        }

        outputs[MIX_OUTPUT + 0].setVoltage(mix & 8 ? 10.f : 0.f);
        outputs[MIX_OUTPUT + 1].setVoltage(mix & 4 ? 10.f : 0.f);
        outputs[MIX_OUTPUT + 2].setVoltage(mix & 2 ? 10.f : 0.f);
        outputs[MIX_OUTPUT + 3].setVoltage(mix & 4 ? 10.f : 0.f);

        lights[MIX_INDICATOR_LIGHT + 0].setBrightness(mix & 8 ? 1.f : 0.f);
        lights[MIX_INDICATOR_LIGHT + 1].setBrightness(mix & 4 ? 1.f : 0.f);
        lights[MIX_INDICATOR_LIGHT + 2].setBrightness(mix & 2 ? 1.f : 0.f);
        lights[MIX_INDICATOR_LIGHT + 3].setBrightness(mix & 1 ? 1.f : 0.f);

        outputs[STEP_OUTPUT].setVoltage(static_cast<float>(mix & 15) * (10.f/16.f));
        lights[STEP_INDICATOR_LIGHT].setBrightness(static_cast<float>(mix & 15) * (1.f/15.f));
    }
};


struct BTMXWidget : ModuleWidget {
	BTMXWidget(BTMX* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BTMX.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 15.033)), module, BTMX::SWITCH_PARAM + 0));
        addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 30.23)), module, BTMX::SWITCH_PARAM + 1));
        addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 45.428)), module, BTMX::SWITCH_PARAM + 2));
        addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 60.626)), module, BTMX::SWITCH_PARAM + 3));
        addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 15.033)), module, BTMX::SWITCH_PARAM + 4));
        addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 30.23)), module, BTMX::SWITCH_PARAM + 5));
        addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 45.597)), module, BTMX::SWITCH_PARAM + 6));
		addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 60.626)), module, BTMX::SWITCH_PARAM + 7));
        addParam(createParamCentered<CKSS>(mm2px(Vec(31.704, 15.033)), module, BTMX::LOGIC_MODE_A));
        addParam(createParamCentered<CKSS>(mm2px(Vec(31.704, 30.23)), module, BTMX::LOGIC_MODE_B));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 72.81)), module, BTMX::IN_INPUT + 0));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 85.794)), module, BTMX::IN_INPUT + 1));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 98.779)), module, BTMX::IN_INPUT + 2));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 111.763)), module, BTMX::IN_INPUT + 3));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 72.81)), module, BTMX::IN_INPUT + 4));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 85.794)), module, BTMX::IN_INPUT + 5));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 98.779)), module, BTMX::IN_INPUT + 6));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 111.763)), module, BTMX::IN_INPUT + 7));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.871, 59.825)), module, BTMX::STEP_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.871, 72.81)), module, BTMX::MIX_OUTPUT + 0));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.871, 85.794)), module, BTMX::MIX_OUTPUT + 1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.703, 98.779)), module, BTMX::MIX_OUTPUT + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.703, 111.763)), module, BTMX::MIX_OUTPUT + 3));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 53.67)), module, BTMX::STEP_INDICATOR_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 66.317)), module, BTMX::IN_INDICATOR_LIGHT + 0));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 79.302)), module, BTMX::IN_INDICATOR_LIGHT + 1));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 92.624)), module, BTMX::IN_INDICATOR_LIGHT + 2));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 105.271)), module, BTMX::IN_INDICATOR_LIGHT + 3));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 66.317)), module, BTMX::IN_INDICATOR_LIGHT + 4));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 79.302)), module, BTMX::IN_INDICATOR_LIGHT + 5));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 92.624)), module, BTMX::IN_INDICATOR_LIGHT + 6));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 105.271)), module, BTMX::IN_INDICATOR_LIGHT + 7));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 66.317)), module, BTMX::MIX_INDICATOR_LIGHT + 0));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 79.302)), module, BTMX::MIX_INDICATOR_LIGHT + 1));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 92.624)), module, BTMX::MIX_INDICATOR_LIGHT + 2));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 105.271)), module, BTMX::MIX_INDICATOR_LIGHT + 3));
	}
};


Model* modelBTMX = createModel<BTMX, BTMXWidget>("BTMX");