fqbn := esp32:esp32:waveshare_esp32_s3_zero
port := /dev/ttyACM0
sketch := mudlogger

update-index:
	arduino-cli core update-index --config-file ./arduino-cli.yaml

compile:
	arduino-cli compile --fqbn $(fqbn) --config-file ./arduino-cli.yaml $(sketch)

upload:
	arduino-cli upload -p $(port) --fqbn $(fqbn) $(sketch)

monitor:
	arduino-cli monitor --config baudrate=9600 -p $(port) --fqbn $(fqbn)

