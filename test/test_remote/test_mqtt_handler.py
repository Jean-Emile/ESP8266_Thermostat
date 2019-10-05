import time
import unittest
import websocket
import json
import paho.mqtt.client as mqtt

WS_URI = 'ws://<IP>:81/'
MQTT_LOGIN = ""
MQTT_PASSWORD = ""
MQTT_SERVER = ""
MQTT_PORT = 1883

class ThermostatWbClient:

    def __init__(self, ws_uri):
        self. ws_uri = ws_uri

    def connect(self):
        self.ws = websocket.WebSocket()
        counter = 0
        while not self.ws.connected and counter < 10:
            try:
                self.ws.connect(self.ws_uri)
            except Exception:
                counter +=1
                self.ws.close()
                time.sleep(1)

    def disconnect(self):
        self.ws.close()

    def get_states(self):
        msg = {"event": "get_thermostat_states"}
        self.ws.send(json.dumps(msg))
        response = self.ws.recv()
        return json.loads(response)

    def change_relay(self, state):
        msg = {"event": "relay", "state": state}
        self.ws.send(json.dumps(msg))

    def change_mode(self, mode):
        msg = {"event": "thermostat_mode", "mode": mode}
        self.ws.send(json.dumps(msg))

    def change_state(self, state):
        msg = {"event": "thermostat_state", "state": state}
        self.ws.send(json.dumps(msg))

    def change_manual_set_point(self, value):
        msg = {"event": "thermostat_manualsetpoint", "temperature": value}
        self.ws.send(json.dumps(msg))

    def change_hysteresis(self, l, h):
        msg = {"event": "set_hysteresis", "hysteresis_h": h, 'hysteresis_l': l}
        self.ws.send(json.dumps(msg))

    def change_selected_sensor(self, sensor_id):
        msg = {"event": "thermostat_selected", "sensorid": sensor_id}
        self.ws.send(json.dumps(msg))

    def change_schedule(self, day, schedule):
        msg = {"event": "thermostat_schedule",
               "day": day,
               "schedule": schedule
               }
        self.ws.send(json.dumps(msg))



class TestThermostatMqtt(unittest.TestCase):
    def setUp(self):
        self.wb_client = ThermostatWbClient(WS_URI)
        self.wb_client.connect()
        self.client = mqtt.Client()
        self.client.username_pw_set(MQTT_LOGIN, MQTT_PASSWORD)
        self.client.connect(MQTT_SERVER, MQTT_PORT)

    def tearDown(self):
        self.client.disconnect()
        self.wb_client.disconnect()

    def test_mode_manual(self):
        self.client.publish("home/thermostat/mode", "manual")
        time.sleep(10)
        self.assertEqual(self.wb_client.get_states()['mode'],0)

        self.client.publish("home/thermostat/mode", "schedule")
        time.sleep(10)
        self.assertEqual(self.wb_client.get_states()['mode'],1)

    def test_manual_setpoint(self):
        expected_value = 1900
        self.client.publish("home/thermostat/manualsetpoint", expected_value)
        time.sleep(10)
        self.assertEqual(self.wb_client.get_states()['manualsetpoint'], expected_value)
