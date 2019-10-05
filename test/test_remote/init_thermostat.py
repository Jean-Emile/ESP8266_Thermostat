import datetime
import time
import unittest
import websocket
import json
import paho.mqtt.client as mqtt


WS_URI = 'ws://<IP>:81/'


class bootStrap(unittest.TestCase):
    days = {
        0: 'mon',
        1: 'tue',
        2: 'wed',
        3: 'thu',
        4: 'fri',
        5: 'sat',
        6: 'sun'
    }

    def setUp(self):
        self.ws = websocket.WebSocket()
        counter = 0
        while not self.ws.connected and counter < 10:
            try:
                self.ws.connect(WS_URI)
            except Exception:
                counter += 1
                self.ws.close()
                time.sleep(1)

    def change_schedule(self, day, schedule):
        msg = {"event": "thermostat_schedule",
               "day": day,
               "schedule": schedule
               }
        self.ws.send(json.dumps(msg))



    def test_init(self):
        schedule = {'5': [{'e': 650, 'sp': 1700, 's': 0}, {'e': 1100, 'sp': 1800, 's': 650},
                            {'e': 1700, 'sp': 1850, 's': 1100},
                            {'e': 2200, 'sp': 1850, 's': 1700}, {'e': 2400, 'sp': 1700, 's': 2200}],
                    '4': [{'e': 650, 'sp': 1700, 's': 0}, {'e': 850, 'sp': 1800, 's': 650},
                            {'e': 1800, 'sp': 1650, 's': 850},
                            {'e': 2200, 'sp': 1850, 's': 1800}, {'e': 2400, 'sp': 1700, 's': 2200}],
                    '1': [{'e': 650, 'sp': 1700, 's': 0}, {'e': 850, 'sp': 1850, 's': 650},
                            {'e': 1800, 'sp': 1650, 's': 850},
                            {'e': 2200, 'sp': 1850, 's': 1800}, {'e': 2400, 'sp': 1700, 's': 2200}],
                    '2': [{'e': 650, 'sp': 1700, 's': 0}, {'e': 850, 'sp': 1850, 's': 650},
                            {'e': 1800, 'sp': 1650, 's': 850},
                            {'e': 2200, 'sp': 1850, 's': 1800}, {'e': 2400, 'sp': 1700, 's': 2200}],
                    '6': [{'e': 650, 'sp': 1700, 's': 0}, {'e': 1100, 'sp': 1800, 's': 650},
                            {'e': 1700, 'sp': 1850, 's': 1100},
                            {'e': 2200, 'sp': 1850, 's': 1700}, {'e': 2400, 'sp': 1700, 's': 2200}],
                    '0': [{'e': 650, 'sp': 1700, 's': 0}, {'e': 850, 'sp': 1850, 's': 650},
                            {'e': 1800, 'sp': 1650, 's': 850},
                            {'e': 2200, 'sp': 1850, 's': 1800}, {'e': 2400, 'sp': 1700, 's': 2200}],
                    '3': [{'e': 650, 'sp': 1700, 's': 0}, {'e': 850, 'sp': 1800, 's': 650},
                            {'e': 1800, 'sp': 1650, 's': 850},
                            {'e': 2200, 'sp': 1850, 's': 1800}, {'e': 2400, 'sp': 1700, 's': 2200}]}

        for day in range(0,7):
            print(day, schedule[str(day)])
            self.change_schedule(day, schedule[str(day)])

        msg = {"event": "get_schedule"}
        self.ws.send(json.dumps(msg))
        response = json.loads(self.ws.recv())
        days2 = {
            0: 'mon',
            1: 'tue',
            2: 'wed',
            3: 'thu',
            4: 'fri',
            5: 'sat',
            6: 'sun'
        }
        for day in self.days:
            self.assertEqual(schedule[str(day)],response[days2[day]])
