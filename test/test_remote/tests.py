import datetime
import time
import unittest
import websocket
import json


class TestThermostatWebSocketApi(unittest.TestCase):

    def setUp(self):
        self.uri = 'ws://casa.powet.eu:65530/'
        self.ws_uri = 'ws://192.168.1.102:81/'
        self.ws = websocket.WebSocket()
        self.ws.connect(self.ws_uri)



    def tearDown(self):
        self.ws.close()

    def get_states(self):
        msg = {"event": "get_thermostat_states"}
        self.ws.send(json.dumps(msg))
        response = self.ws.recv()
        return response

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

    def test_change_state(self):
        # MANUAL
        off = 0
        self.change_state(off)
        self.assertEqual(json.loads(self.get_states())['state'], off)

        on = 1
        self.change_state(on)
        self.assertEqual(json.loads(self.get_states())['state'], on)

        off = 0
        self.change_state(off)
        self.assertEqual(json.loads(self.get_states())['state'], off)

        on = 1
        self.change_state(on)
        self.assertEqual(json.loads(self.get_states())['state'], on)

    def test_change_mode(self):
        # MANUAL
        set_manual = 0
        self.change_mode(set_manual)
        self.assertEqual(json.loads(self.get_states())['mode'], set_manual)

        set_schedule = 1
        self.change_mode(set_schedule)
        self.assertEqual(json.loads(self.get_states())['mode'], set_schedule)

        set_auto = 2
        self.change_mode(set_auto)
        self.assertEqual(json.loads(self.get_states())['mode'], set_auto)

        set_schedule = 1
        self.change_mode(set_schedule)
        self.assertEqual(json.loads(self.get_states())['mode'], set_schedule)

    def test_hysteresis(self):
        expected_h = 25
        expected_l = 25
        self.change_hysteresis(expected_h, expected_l)
        self.assertEqual(json.loads(self.get_states())['hysteresis_h'], expected_h)
        self.assertEqual(json.loads(self.get_states())['hysteresis_l'], expected_l)

        expected_h = 50
        expected_l = 50
        self.change_hysteresis(expected_h, expected_l)
        self.assertEqual(json.loads(self.get_states())['hysteresis_h'], expected_h)
        self.assertEqual(json.loads(self.get_states())['hysteresis_l'], expected_l)

        expected_h = 25
        expected_l = 25
        self.change_hysteresis(expected_h, expected_l)
        self.assertEqual(json.loads(self.get_states())['hysteresis_h'], expected_h)
        self.assertEqual(json.loads(self.get_states())['hysteresis_l'], expected_l)

    def test_manualsetpoint(self):
        expected_value = 1900
        self.change_manual_set_point(expected_value)
        self.assertEqual(json.loads(self.get_states())['manualsetpoint'], expected_value)
        expected_value = 2100
        self.change_manual_set_point(expected_value)
        self.assertEqual(json.loads(self.get_states())['manualsetpoint'], expected_value)

        expected_value = 1815
        self.change_manual_set_point(expected_value)
        self.assertEqual(json.loads(self.get_states())['manualsetpoint'], expected_value)

    def test_change_sensor_id(self):
        expected_value = 1
        self.change_selected_sensor(expected_value)
        self.assertEqual(json.loads(self.get_states())['selected'], expected_value)
        expected_value = 0
        self.change_selected_sensor(expected_value)
        self.assertEqual(json.loads(self.get_states())['selected'], expected_value)
        expected_value = 1
        self.change_selected_sensor(expected_value)
        self.assertEqual(json.loads(self.get_states())['selected'], expected_value)
        expected_value = 10
        self.change_selected_sensor(expected_value)
        self.assertEqual(json.loads(self.get_states())['selected'], expected_value)

    def test_relay(self):
        expected_value = False
        self.change_relay(expected_value)
        self.assertEqual(json.loads(self.get_states())['relayState'], False)
        time.sleep(0.30)
        expected_value = True
        self.change_relay(expected_value)
        self.assertEqual(json.loads(self.get_states())['relayState'], True)
        time.sleep(0.30)
        expected_value = False
        self.change_relay(expected_value)
        self.assertEqual(json.loads(self.get_states())['relayState'], False)
        time.sleep(0.30)
        expected_value = True
        self.change_relay(expected_value)
        self.assertEqual(json.loads(self.get_states())['relayState'], True)
        time.sleep(0.30)
        expected_value = False
        self.change_relay(expected_value)
        self.assertEqual(json.loads(self.get_states())['relayState'], False)

    def test_get_time(self):
        msg = {"event": "time"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())['currtime']

        timeformat = "%H:%M:%S"
        d = datetime.datetime.strptime(response, timeformat)

        self.assertEqual(d.hour, datetime.datetime.now().hour)
        self.assertEqual(d.minute - datetime.datetime.now().minute < 1, True)
        self.assertEqual(d.second - datetime.datetime.now().second < 2, True)

    def test_do_wifi_scan(self):
        msg = {"event": "do_wifi_scan"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())
        self.assertEqual(response['event'], 'wifi_scan')


    def test_schedule_errors(self):
        days = {
            0: 'mon',
            1: 'tue',
            2: 'wed',
            3: 'thu',
            4: 'fri',
            5: 'sat',
            6: 'sun'
        }

        schedule3 = [{"s": 0, "e": 100, "sp": 1190},
                     {"s": 100, "e": 150, "sp": 1000},
                     {"s": 150, "e": 300, "sp": 1800},
                     {"s": 300, "e": 650, "sp": 1850},
                     {"s": 650, "e": 1150, "sp": 600},
                     {"s": 1150, "e": 1400, "sp": 1600},
                     {"s": 1400, "e": 1700, "sp": 3300},
                     {"s": 1700, "e": 2400, "sp": 2100},
                     {"s": 2200, "e": 2400, "sp": 0}]

        for day in days:
            self.change_schedule(day, schedule3)

        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())
        for day in days:
            self.assertNotEqual(schedule3, response[days[day]])


        schedule = [{"s": 0, "e": 450},
                    {"s": 450, "e": 1000, "sp": 1800},
                    {"s": 1000, "e": 1700, "sp": 1600},
                    {"s": 1700, "e": 2200, "sp": 2100},
                    {"s": 2200, "e": 2400, "sp": 1500}]

        for day in days:
            self.change_schedule(day, schedule)

        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())
        for day in days:
            self.assertNotEqual(schedule, response[days[day]])


    def test_schedule_backup(self):
        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        expected = json.loads(self.ws.recv())

        msg = {"event": "restart"}
        self.ws.send(json.dumps(msg))

        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())

        self.assertEqual(response, expected)


    def test_schedule(self):
        days = {
            0: 'mon',
            1: 'tue',
            2: 'wed',
            3: 'thu',
            4: 'fri',
            5: 'sat',
            6: 'sun'
        }
        schedule = [{"s": 0, "e": 450, "sp": 1000},
                    {"s": 450, "e": 1000, "sp": 1800},
                    {"s": 1000, "e": 1700, "sp": 1600},
                    {"s": 1700, "e": 2200, "sp": 2100},
                    {"s": 2200, "e": 2400, "sp": 1500}]

        for day in days:
            self.change_schedule(day, schedule)

        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())
        for day in days:
            self.assertEqual(schedule, response[days[day]])

        schedule2 = [{"s": 0, "e": 250, "sp": 1000},
                     {"s": 250, "e": 450, "sp": 1000},
                     {"s": 450, "e": 650, "sp": 1800},
                     {"s": 650, "e": 1000, "sp": 1800},
                     {"s": 1000, "e": 1700, "sp": 1600},
                     {"s": 1700, "e": 2200, "sp": 2100},
                     {"s": 2200, "e": 2400, "sp": 1500}]

        for day in days:
            self.change_schedule(day, schedule2)




        schedule3 = [{"s": 0, "e": 100, "sp": 1190},
                     {"s": 100, "e": 150, "sp": 1000},
                     {"s": 150, "e": 300, "sp": 1800},
                     {"s": 300, "e": 650, "sp": 1850},
                     {"s": 650, "e": 1150, "sp": 600},
                     {"s": 1150, "e": 1400, "sp": 1600},
                     {"s": 1400, "e": 1700, "sp": 3300},
                     {"s": 1700, "e": 2400, "sp": 2100}]

        for day in days:
            self.change_schedule(day, schedule3)

        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())
        for day in days:
            self.assertEqual(schedule3, response[days[day]])


        days = {
            0: 'mon',
            1: 'tue',
            2: 'wed',
            3: 'thu',
            4: 'fri',
            5: 'sat',
            6: 'sun'
        }
        schedule = [{"s": 0, "e": 450, "sp": 1000},
                    {"s": 450, "e": 1000, "sp": 1800},
                    {"s": 1000, "e": 1700, "sp": 1600},
                    {"s": 1700, "e": 2200, "sp": 2100},
                    {"s": 2200, "e": 2400, "sp": 1500}]

        for day in days:
            self.change_schedule(day, schedule)

        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())
        for day in days:
            self.assertEqual(schedule, response[days[day]])

if __name__ == '__main__':
    unittest.main()
