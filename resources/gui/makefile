
all: gui.js gui.css

gui.js: gui.elm
ifeq ($(NIX_BUILD_TOP),)
	elm make --output=$@ $<
else
	echo "calling elm-make with custom HOME:" $(TMP)
	HOME=$(TMP) elm make --output=$@ $<
endif

gui.css: gui.scss
	sass --scss $< $@
