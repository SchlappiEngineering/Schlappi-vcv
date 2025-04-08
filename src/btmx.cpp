#include "plugin.hpp"
#include <rack.hpp>


struct BTMX : Module {
	enum ParamId {
		SWITCH_1_PARAM,
		SWITCH_5_PARAM,
		SWITCH_2_PARAM,
		SWITCH_6_PARAM,
		SWITCH_3_PARAM,
		SWITCH_7_PARAM,
		SWITCH_4_PARAM,
		SWITCH_8_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN_1_INPUT,
		IN_5_INPUT,
		IN_2_INPUT,
		IN_6_INPUT,
		IN_3_INPUT,
		IN_7_INPUT,
		IN_4_INPUT,
		IN_8_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		STEP_OUTPUT,
		MIX_1_5_OUTPUT,
		MIX_2_6_OUTPUT,
		MIX_3_7_OUTPUT,
		MIX_4_8_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		STEP_INDICATOR_LIGHT,
		IN_1_INDICATOR_LIGHT,
		IN_5_INDICATOR_LIGHT,
		MIX_1_5_INDICATOR_LIGHT,
		IN_2_INDICATOR_LIGHT,
		IN_6_INDICATOR_LIGHT,
		MIX_2_6_INDICATOR_LIGHT,
		IN_3_INDICATOR_LIGHT,
		IN_7_INDICATOR_LIGHT,
		MIX_3_7_INDICATOR_LIGHT,
		IN_4_INDICATOR_LIGHT,
		IN_8_INDICATOR_LIGHT,
		MIX_4_8_INDICATOR_LIGHT,
		LIGHTS_LEN
	};

    T

	BTMX() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SWITCH_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SWITCH_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SWITCH_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SWITCH_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SWITCH_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SWITCH_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SWITCH_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SWITCH_8_PARAM, 0.f, 1.f, 0.f, "");
		configInput(IN_1_INPUT, "");
		configInput(IN_5_INPUT, "");
		configInput(IN_2_INPUT, "");
		configInput(IN_6_INPUT, "");
		configInput(IN_3_INPUT, "");
		configInput(IN_7_INPUT, "");
		configInput(IN_4_INPUT, "");
		configInput(IN_8_INPUT, "");
		configOutput(STEP_OUTPUT, "");
		configOutput(MIX_1_5_OUTPUT, "");
		configOutput(MIX_2_6_OUTPUT, "");
		configOutput(MIX_3_7_OUTPUT, "");
		configOutput(MIX_4_8_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
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

		addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 15.033)), module, BTMX::SWITCH_1_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 15.033)), module, BTMX::SWITCH_5_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 30.23)), module, BTMX::SWITCH_2_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 30.23)), module, BTMX::SWITCH_6_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 45.428)), module, BTMX::SWITCH_3_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 45.597)), module, BTMX::SWITCH_7_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(5.733, 60.626)), module, BTMX::SWITCH_4_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(18.718, 60.626)), module, BTMX::SWITCH_8_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 72.81)), module, BTMX::IN_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 72.81)), module, BTMX::IN_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 85.794)), module, BTMX::IN_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 85.794)), module, BTMX::IN_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 98.779)), module, BTMX::IN_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 98.779)), module, BTMX::IN_7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.733, 111.763)), module, BTMX::IN_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.718, 111.763)), module, BTMX::IN_8_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.871, 59.825)), module, BTMX::STEP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.871, 72.81)), module, BTMX::MIX_1_5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.871, 85.794)), module, BTMX::MIX_2_6_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.703, 98.779)), module, BTMX::MIX_3_7_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.703, 111.763)), module, BTMX::MIX_4_8_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 53.67)), module, BTMX::STEP_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 66.317)), module, BTMX::IN_1_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 66.317)), module, BTMX::IN_5_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 66.317)), module, BTMX::MIX_1_5_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 79.302)), module, BTMX::IN_2_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 79.302)), module, BTMX::IN_6_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 79.302)), module, BTMX::MIX_2_6_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 92.624)), module, BTMX::IN_3_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 92.624)), module, BTMX::IN_7_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 92.624)), module, BTMX::MIX_3_7_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.057, 105.271)), module, BTMX::IN_4_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.21, 105.271)), module, BTMX::IN_8_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.026, 105.271)), module, BTMX::MIX_4_8_INDICATOR_LIGHT));
	}
};


Model* modelBTMX = createModel<BTMX, BTMXWidget>("BTMX");