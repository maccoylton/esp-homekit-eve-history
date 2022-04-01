# Component makefile for esp-homekit-eve-history


INC_DIRS += $(esp-homekit-eve-history_ROOT)


esp-homekit-eve-history_INC_DIR = $(esp-homekit-eve-history_ROOT)
esp-homekit-eve-history_SRC_DIR = $(esp-homekit-eve-history_ROOT)

$(eval $(call component_compile_rules,esp-homekit-eve-history))
