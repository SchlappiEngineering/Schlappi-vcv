#include "plugin.hpp"


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
	}

	void process(const ProcessArgs& args) override {
	}
};


struct NibblerWidget : ModuleWidget {
	NibblerWidget(Nibbler* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Nibbler.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.494, 16.384)), module, Nibbler::ADD_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(49.512, 16.384)), module, Nibbler::OFFSET_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 24.311)), module, Nibbler::RESET_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.494, 31.629)), module, Nibbler::ADD_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(49.512, 31.629)), module, Nibbler::OFFSET_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23.5, 41.595)), module, Nibbler::SUBTRACT_ADD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.506, 41.595)), module, Nibbler::ASYNC_SYNC_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.494, 46.831)), module, Nibbler::ADD_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.494, 62.075)), module, Nibbler::ADD_1_PARAM));

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

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(55.53, 40.666)), module, Nibbler::OFFSET_STEP_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(42.861, 53.503)), module, Nibbler::STEP_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(55.53, 53.503)), module, Nibbler::CARRY_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(29.855, 66.171)), module, Nibbler::CARRY_IN_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(42.861, 66.171)), module, Nibbler::GATE_8_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(55.53, 66.171)), module, Nibbler::OUT_8_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(16.849, 79.178)), module, Nibbler::CLOCK_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(29.855, 79.178)), module, Nibbler::SHIFT_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(42.861, 79.178)), module, Nibbler::GATE_4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(55.868, 79.178)), module, Nibbler::OUT_4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(16.849, 92.522)), module, Nibbler::RESET_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(55.868, 92.184)), module, Nibbler::OUT_2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(29.855, 92.522)), module, Nibbler::SHIFT_DATA_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(42.861, 92.522)), module, Nibbler::GATE_2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(16.849, 105.19)), module, Nibbler::SUB_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(29.855, 105.19)), module, Nibbler::DATA_XOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(42.861, 105.19)), module, Nibbler::GATE_1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(55.868, 105.19)), module, Nibbler::OUT_1_LIGHT));
	}
};


Model* modelNibbler = createModel<Nibbler, NibblerWidget>("Nibbler");