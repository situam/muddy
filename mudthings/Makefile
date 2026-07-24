fqbn := esp32:esp32:waveshare_esp32_s3_zero
# fqbn := esp32:esp32:XIAO_ESP32C3
port := /dev/ttyACM0
sketch := mudlogger
# sketch := testmux
# sketch := testfs

go:
	make compile
	make upload
	make monitor

update-index:
	arduino-cli core update-index --config-file ./arduino-cli.yaml

compile:
	arduino-cli compile \
		--fqbn $(fqbn) \
		--config-file ./arduino-cli.yaml \
		$(sketch)

upload:
	sudo chmod 777 $(port)
	arduino-cli upload -p $(port) --fqbn $(fqbn) $(sketch)

monitor:
	sudo chmod 777 $(port)
	arduino-cli monitor --config baudrate=115200 -p $(port) --fqbn $(fqbn)

