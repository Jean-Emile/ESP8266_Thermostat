// code adapted from Martin Harizanov project (https://github.com/mharizanov/ESP8266_Relay_Board)
var now = new Date();
var timenow = now.getHours() + (now.getMinutes() / 60);
var days = {
    0: 'sun',
    1: 'mon',
    2: 'tue',
    3: 'wed',
    4: 'thu',
    5: 'fri',
    6: 'sat',
    7: 'sun'
};
// FIXME
function getName(id) {
    if(id == 6){
        return "Elisa"
    }else if(id == 7){
        return "Salon"
    }else if(id == 0){
        return "Cuisine";
    }else
    {
        return "Node_"+id;
    }

}

function setState(msg) {
    var connectionState = document.getElementById("connectionState");
    connectionState.innerHTML =msg;
}
var today = days[now.getDay()];

var wb_url = 'ws://casa.powet.eu:65530/'; // esp8266.local

var connection = new WebSocket(wb_url);
connection.onopen = function () {
    console.log("connected...");
    updateclock();
    setState("connected");

};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
    reconnect();
};
connection.onmessage = function (e) {
    console.log('Server: ', e.data);


    var json = JSON.parse(e.data);
    // json =  {"event":"thermostat_schedule",
    //     "mon":[{"s":0,"e":400,"sp":1000},{"s":400,"e":1100,"sp":1800},{"s":1100,"e":1700,"sp":1600},{"s":1700,"e":2200,"sp":2100},{"s":2200,"e":2400,"sp":1500}],
    //     "tue":[{"s":0,"e":600,"sp":1000},{"s":600,"e":900,"sp":1800},{"s":900,"e":1700,"sp":1600},{"s":1700,"e":2200,"sp":2100},{"s":2200,"e":2400,"sp":1500}],
    //     "wed":[{"s":0,"e":600,"sp":1000},{"s":600,"e":900,"sp":1800},{"s":900,"e":1700,"sp":1600},{"s":1700,"e":2200,"sp":2100},{"s":2200,"e":2400,"sp":1500}],
    //     "thu":[{"s":0,"e":600,"sp":1000},{"s":600,"e":900,"sp":1800},{"s":900,"e":1700,"sp":1600},{"s":1700,"e":2200,"sp":2100},{"s":2200,"e":2400,"sp":1500}],
    //     "fri":[{"s":0,"e":600,"sp":1000},{"s":600,"e":900,"sp":1800},{"s":900,"e":1700,"sp":1600},{"s":1700,"e":2200,"sp":2100},{"s":2200,"e":2300,"sp":2000},{"s":2300,"e":2400,"sp":1900}],
    //     "sat":[{"s":0,"e":600,"sp":1000},{"s":600,"e":900,"sp":1800},{"s":900,"e":1700,"sp":1600},{"s":1700,"e":1950,"sp":2100},{"s":1950,"e":2200,"sp":1500},{"s":2200,"e":2400,"sp":1500}],
    //     "sun":[{"s":0,"e":600,"sp":1000},{"s":600,"e":900,"sp":1800},{"s":900,"e":1700,"sp":1600},{"s":1700,"e":2200,"sp":2100},{"s":2200,"e":2400,"sp":1500}]}

    if(json["event"] == "thermostat_states"){
        delete json["event"];
        thermostat = json;
        thermostat.manualsetpoint/=100;

    }else if(json["event"] == "thermostat_schedule")
    {

        delete json["event"];
        schedule = json;

        for (var d in schedule) {
            for (var z in schedule[d]) {
                schedule[d][z].s /= 100;
                schedule[d][z].e /= 100;
                schedule[d][z].sp /= 100;
                //console.log(d+" "+z+" "+schedule[d][z].s+" "+schedule[d][z].e+" "+schedule[d][z].sp)
                draw_day_slider(d)
            }
        }


    }
    update();
};
connection.onclose = function(){
    console.log('WebSocket connection closed');
    reconnect();
};

autoReconnectInterval = 1*1000

function reconnect(){
    setTimeout(function(){
        console.log("reconnecting...");
        setState("reconnecting...");
        connection = new WebSocket(wb_url);
    },autoReconnectInterval);
}



//=================================================
// DATA
//=================================================

var visibleFlag = 1;
var unit ="&deg;C";
var statusMsg = false;
var connected = false;
var doingsave = false;

var thermostat = {
    temperature: 0,
    humidity: 0,
    sensors: {},
    relayState: false,
    opmode: 0,
    state: false,
    manualsetpoint: 0,
    mode: 0,
    boardtime: "",
    selected: 0,
    hysteresis_h:50,
    hysteresis_l:50
};

var schedule = {};

var maxc = 24;
var minc = 5;
// ================================================
// State variables
// ================================================
var editmode = 'move';
$("#mode-move").css("background-color", "#ff9600");
var key = 1;
var day = "mon";
var mousedown = 0;
var slider_width = $(".slider").width();
var slider_height = $(".slider").height();
var changed = 0;


$("#unit").html(unit);

setpoint = thermostat.manualsetpoint;
$(".zone-setpoint").html(setpoint.toFixed(1) + unit);

update();
updateclock();
setInterval(updateclock, 5000);


function requestBoardStates() {
    var cmd = new Object();
    cmd.event = "get_thermostat_states";
    var json = JSON.stringify(cmd);
    connection.send(json);
}
function updateclock() {
    if(connection.readyState === WebSocket.OPEN){

        requestBoardStates();
    }


}


function update() {
    if(connection.readyState === WebSocket.OPEN){

        if(!isNaN((Number(thermostat.humidity)).toFixed(1)))
            $('.humidity').show();


        if(thermostat.relayState === false) {
            $(".zone-setpoint").css("color", "#000000");
        } else {
            $(".zone-setpoint").css("color", "#f00000");
        }

        if (thermostat.state === 1) {
            $("#toggle").html("ON");
            $("#toggle").css("background-color", "#ff9600");
        } else {
            $("#toggle").html("OFF");
            $("#toggle").css("background-color", "#555");
        }

        if (thermostat.mode === 0) {
            $(".thermostatmode").css("background-color", "#555");
            $("#manual_thermostat").css("background-color", "#ff9600");
            $("#scheduled_thermostat").css("background-color", "#555");
            $("#scheduled_auto").css("background-color", "#555");
        } else if(thermostat.mode === 1) {
            $(".thermostatmode").css("background-color", "#555");
            $("#manual_thermostat").css("background-color", "#555");
            $("#scheduled_thermostat").css("background-color", "#ff9600");
            $("#scheduled_auto").css("background-color", "#555");
        } else if(thermostat.mode === 2) {
            $(".thermostatmode").css("background-color", "#555");
            $("#manual_thermostat").css("background-color", "#555");
            $("#scheduled_thermostat").css("background-color", "#555");
            $("#scheduled_auto").css("background-color", "#ff9600");
        }

        if (thermostat.opmode == 0) {
            $(".thermostatopmode").css("background-color", "#555");
            $("#heating_thermostat").css("background-color", "#c00000");
            $("#cooling_thermostat").css("background-color", "#555");
        } else {
            $(".thermostatopmode").css("background-color", "#555");
            $("#heating_thermostat").css("background-color", "#555");
            $("#cooling_thermostat").css("background-color", "#0000c0");
        }

        now = new Date();
        timenow = now.getHours() + (now.getMinutes() / 60);
        today = days[now.getDay()];

        checkVisibility();


        if (thermostat.mode == 0) {
            setpoint = thermostat.manualsetpoint;
            $(".zone-setpoint").html(setpoint.toFixed(1) + unit);
        }

        var aps = document.getElementById("boardtime");
        aps.innerHTML =    "Board Time: "+thermostat.boardtime;


        var thermostat1hysteresishigh = document.getElementById("thermostat1hysteresishigh");
        thermostat1hysteresishigh.value = thermostat.hysteresis_h;

        var thermostat1hysteresislow = document.getElementById("thermostat1hysteresislow");
        thermostat1hysteresislow.value = thermostat.hysteresis_l;


        var aps = document.getElementById("sensors");
        while(aps.firstChild){
            aps.removeChild(aps.firstChild);
        }

        for(var i in thermostat.sensors){
            var sensor =  thermostat.sensors[i];

            var sensorUpdate = new Date(0); // The 0 there is the key, which sets the date to the epoch
            sensorUpdate.setUTCSeconds(sensor.lastUp);
            sensorUpdate.setTime( sensorUpdate.getTime() + sensorUpdate.getTimezoneOffset()*60*1000 );

            var difference =now.getTime() - sensorUpdate.getTime(); // This will give difference in milliseconds
            var resultInSeconds = Math.round(difference / 6000);

            var div=document.createElement("div");
           div.setAttribute("align","left");

            var checkbox = document.createElement('input');
            checkbox.type = "checkbox";
            checkbox.onclick = function(){
               // this.onclick = null;
                //var label = this.parentNode;
               // label.removeChild(this);
                //label.parentNode.removeChild(label);
                var cmd = new Object();
                cmd.event = "thermostat_selected";
                cmd["sensorid"] = parseInt(this.value);
                var json = JSON.stringify(cmd);
                connection.send(json);
                console.log(cmd);

                requestBoardStates();

            };
            checkbox.id = "s_"+sensor.id;
            checkbox.value = sensor.id;

            var label = document.createElement('label');
            label.for = "s_"+sensor.id;
            label.innerHTML = ""+getName(sensor.id)+" Temperature: "+((Number(sensor.temp/100)).toFixed(1) + " Humidiy: "+
                (Number(sensor.hum/100)).toFixed(1))+" % ("+resultInSeconds+" seconds) "+sensor.batt/100+"V RSSI "+sensor.rssi;


            if(thermostat.selected == sensor.id){
                checkbox.checked = true;
            }

            div.appendChild(checkbox);
            div.appendChild(label);

            aps.appendChild(div);

        }



        var current_key = 0;
        for (var z in schedule[today]) {
            if (schedule[today][z].s <= timenow && schedule[today][z].e > timenow) {
                if (thermostat.mode == 1) {
                    setpoint = schedule[today][z].sp * 1;
                    $(".zone-setpoint").html(setpoint.toFixed(1) + unit);
                    current_key = z;
                }
            }

        }



        var sx = $(".slider[day=" + today + "]")[0].offsetLeft;
        var y = $(".slider[day=" + today + "]")[0].offsetTop;
        var x1 = sx + slider_width * (timenow / 24.0);
        var x2 = sx + slider_width * (schedule[today][current_key].s / 24.0);

        x2 = sx;
        $("#timemarker").css('top', y + "px");
        $("#timemarker").css('left', x2 + "px");
        $("#timemarker").css('width', (x1 - x2) + "px");
    }
}

$("#toggle").click(function () {

    if (thermostat.state == 0) {
        thermostat.state = 1;
        $("#toggle").html("ON");
        $(this).css("background-color", "#ff9600");
    }else {
        thermostat.state =0;
        $("#toggle").html("OFF");
        $(this).css("background-color", "#555");
    }

    save("thermostat_state", thermostat.state);
});

$("#zone-setpoint-dec").click(function () {
    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#ff9600");
    thermostat.mode = 0;
    thermostat.manualsetpoint -= 0.5;
    setpoint = thermostat.manualsetpoint;
    $(".zone-setpoint").html(setpoint.toFixed(1) + unit);

    //save("tx/heating",thermostat.state+","+parseInt(setpoint*100));
    save("thermostat_mode", thermostat.mode);
    save("thermostat_manualsetpoint", ((thermostat.manualsetpoint.toFixed(1)) * 100));
});

$("#zone-setpoint-inc").click(function () {
    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#ff9600");
    thermostat.mode = 0;
    thermostat.manualsetpoint += 0.5;
    setpoint = thermostat.manualsetpoint;
    $(".zone-setpoint").html(setpoint.toFixed(1) + unit);

    //save("tx/heating",thermostat.state+","+parseInt(setpoint*100));
    save("thermostat_mode", thermostat.mode);
    save("thermostat_manualsetpoint", ((thermostat.manualsetpoint.toFixed(1)) * 100));
});

// ============================================
// SCHEDULER

for (day in schedule) draw_day_slider(day);

function draw_day_slider(day) {
    var out = "";
    var key = 0;
    for (var z in schedule[day]) {
        var left = (schedule[day][z].s / 24.0) * 100;
        var width = ((schedule[day][z].e - schedule[day][z].s) / 24.0) * 100;
        var color = color_map(schedule[day][z].sp);

        out += "<div class='slider-segment' style='left:" + left + "%; width:" + width + "%; background-color:" + color + "' key=" + key + " title='" + schedule[day][z].sp + unit +"'></div>";

        if (key > 0) {
            out += "<div class='slider-button' style='left:" + left + "%;' key=" + key + "></div>";
        }
        key++;
    }
    out += "<div class='slider-label'>" + day.toUpperCase() + "</div>";
    $(".slider[day=" + day + "]").html(out);
}
$("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));

$("body").on("mousedown", ".slider-button", function (e) {
    mousedown = 1;
    key = $(this).attr('key');
});
$("body").mouseup(function (e) {
    mousedown = 0;
    if (changed) {
        save("thermostat_schedule", day);
        changed = 0;
    }
});

$("body").on("mousemove", ".slider", function (e) {
    if (mousedown && editmode == 'move') {
        day = $(this).attr('day');
        slider_update(e);
    }
});

$("body").on("touchstart", ".slider-button", function (e) {
    mousedown = 1;
    key = $(this).attr('key');
});
$("body").on("touchend", ".slider-button", function (e) {
    mousedown = 0;
    if (changed) {
        save("thermostat_schedule", day);
        changed = 0;
    }
});

$("body").on("touchmove", ".slider", function (e) {

    var event = window.event;
    e.pageX = event.touches[0].pageX;
    if (mousedown && editmode == 'move') {
        day = $(this).attr('day');
        slider_update(e);
    }
});

// MERGE
$("body").on("click", ".slider-button", function () {
    if (editmode == 'merge') {
        day = $(this).parent().attr("day");
        key = parseInt($(this).attr("key"),10);
        schedule[day][key - 1].e = schedule[day][key].e;
        schedule[day].splice(key, 1);
        draw_day_slider(day);
        //editmode = 'move';
        save("thermostat_schedule", day);
    }
});

$("body").on("click", ".slider-segment", function (e) {

    day = $(this).parent().attr("day");
    key = parseInt($(this).attr("key"),10);

    if (editmode == 'split') {
        var x = e.pageX - $(this).parent()[0].offsetLeft;
        var prc = x / slider_width;
        var hour = prc * 24.0;
        hour = Math.round(hour / 0.5) * 0.5;

        if (hour > schedule[day][key].s + 0.5 && hour < schedule[day][key].e - 0.5) {
            var end = parseFloat(schedule[day][key].e);
            schedule[day][key].e = hour;

            schedule[day].splice(key + 1, 0, {
                s: hour,
                e: end,
                sp: schedule[day][key].sp
            });

            draw_day_slider(day);
            $("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));
            save("thermostat_schedule", day);
        }
        //editmode = 'move';
    } else if (editmode == 'move') {
        $("#slider-segment-temperature").val((schedule[day][key].sp * 1).toFixed(1));
        $("#slider-segment-start").val(format_time(schedule[day][key].s));
        $("#slider-segment-end").val(format_time(schedule[day][key].e));

        $("#slider-segment-block").show();
        $("#slider-segment-block-movepos").hide();
    }
});

function slider_update(e) {
    $("#slider-segment-block-movepos").show();
    $("#slider-segment-block").hide();

    if (key !== undefined) {
        var x = e.pageX - $(".slider[day=" + day + "]")[0].offsetLeft;

        var prc = x / slider_width;
        var hour = prc * 24.0;
        hour = Math.round(hour / 0.5) * 0.5;

        if (hour > schedule[day][key - 1].s && hour < schedule[day][key].e) {
            schedule[day][key - 1].e = hour;
            schedule[day][key].s = hour;
            update_slider_ui(day, key);
            changed = 1;
        }
        $("#slider-segment-time").val(format_time(schedule[day][key].s));
    }
    // $("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));


}

$("body").on("click", "#slider-segment-ok", function () {

    schedule[day][key].sp = $("#slider-segment-temperature").val();
    var color = color_map(schedule[day][key].sp);
    $(".slider[day=" + day + "]").find(".slider-segment[key=" + key + "]").css("background-color", color);

    var time = decode_time($("#slider-segment-start").val());
    if (time != -1 && key > 0 && key < schedule[day].length) {
        if (time >= (schedule[day][key - 1].s + 0.5) && time <= (schedule[day][key].e - 0.5)) {
            schedule[day][key - 1].e = time;
            schedule[day][key].s = time;
        }
    }
    $("#slider-segment-start").val(format_time(schedule[day][key].s));
    update_slider_ui(day, key);

    time = decode_time($("#slider-segment-end").val());
    if (time != -1 && key > 0 && key < (schedule[day].length - 1)) {
        if (time >= (schedule[day][key].s + 0.5) && time <= (schedule[day][key + 1].e - 0.5)) {
            schedule[day][key].e = time;
            schedule[day][key + 1].s = time;
        }
    }
    $("#slider-segment-end").val(format_time(schedule[day][key].e));
    update_slider_ui(day, key + 1);
    save("thermostat_schedule", day);
    updateclock();

});

$("#slider-segment-movepos-ok").click(function () {

    var time = decode_time($("#slider-segment-time").val());
    if (time != -1 && key > 0) {
        if (time >= (schedule[day][key - 1].s + 0.5) && time <= (schedule[day][key].e - 0.5)) {
            schedule[day][key - 1].e = time;
            schedule[day][key].s = time;
        }
    }
    $("#slider-segment-time").val(format_time(schedule[day][key].s));
    update_slider_ui(day, key);
    save("thermostat_schedule", day);
});

$("#mode-split").click(function () {
    editmode = 'split';
    $(".editmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
});


$("#mode-move").click(function () {
    editmode = 'move';
    $(".editmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
});

$("#mode-merge").click(function () {
    editmode = 'merge';
    $(".editmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
});

$("#manual_thermostat").click(function () {
    $(".thermostatmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
    thermostat.mode = 0;

    setpoint = thermostat.manualsetpoint;
    $(".zone-setpoint").html(setpoint.toFixed(1) + unit);

    save("thermostat_mode", thermostat.mode);
    updateclock();
});

$("#scheduled_thermostat").click(function () {
    $(".thermostatmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
    thermostat.mode = 1;
    save("thermostat_mode", (thermostat.mode).toString());
    updateclock();
});

$("#scheduled_auto").click(function () {
    console.log("mode auto")
    $(".thermostatmode").css("background-color", "#555");
    $(this).css("background-color", "#ff9600");
    thermostat.mode = 2;
    save("thermostat_mode", (thermostat.mode).toString());
    updateclock();
});

$("#heating_thermostat").click(function () {
    $(".thermostatopmode").css("background-color", "#555");
    $(this).css("background-color", "#c00000");
    opmode = 0;
    save("thermostat_opmode", opmode.toString());
    updateclock();
});

$("#cooling_thermostat").click(function () {
    $(".thermostatopmode").css("background-color", "#555");
    $(this).css("background-color", "#0000c0");
    opmode = 1;
    save("thermostat_opmode", opmode.toString());
    updateclock();
});

function color_map(temperature) {
    /*
    // http://www.particleincell.com/blog/2014/colormap/
    // rainbow short
    var f=(temperature-minc)/(maxc-minc);	//invert and group
	var a=(1-f)/0.25;	//invert and group
	var X=Math.floor(a);	//this is the integer part
	var Y=Math.floor(255*(a-X)); //fractional part from 0 to 255
	switch(X)
	{
		case 0: r=255;g=Y;b=0;break;
		case 1: r=255-Y;g=255;b=0;break;
		case 2: r=0;g=255;b=Y;break;
		case 3: r=0;g=255-Y;b=255;break;
		case 4: r=0;g=0;b=255;break;
	}

	*/
    var f = (temperature - minc) / (maxc - minc);
    var a = (1 - f);
    var Y = Math.floor(255 * a);
    r = 255;
    g = Y;
    b = 0;

    return "rgb(" + r + "," + g + "," + b + ")";
}

function update_slider_ui(day, key) {
    if (schedule[day] !== undefined && key < schedule[day].length) {
        var slider = $(".slider[day=" + day + "]");
        if (key > 0) {
            var width = ((schedule[day][key - 1].e - schedule[day][key - 1].s) / 24.0) * 100;
            slider.find(".slider-segment[key=" + (key - 1) + "]").css("width", width + "%");
        }

        var left = (schedule[day][key].s / 24.0) * 100;
        var width = ((schedule[day][key].e - schedule[day][key].s) / 24.0) * 100;
        slider.find(".slider-segment[key=" + key + "]").css("width", width + "%");
        slider.find(".slider-segment[key=" + key + "]").css("left", left + "%");
        slider.find(".slider-button[key=" + key + "]").css("left", left + "%");
    }
}

function format_time(time) {
    var hour = Math.floor(time);
    var mins = Math.round((time - hour) * 60);
    if (mins < 10) mins = "0" + mins;
    return hour + ":" + mins;
}

function decode_time(timestring) {
    var time = -1;
    if (timestring.indexOf(":") != -1) {
        var parts = timestring.split(":");
        var hour = parseInt(parts[0],10);
        var mins = parseInt(parts[1],10);

        if (mins >= 0 && mins < 60 && hour >= 0 && hour < 25) {
            if (hour == 24 && mins !== 0) {} else {
                time = hour + (mins / 60);
            }
        }
    }
    return time;
}

function calc_average_schedule_temperature() {
    var sum = 0;
    for (var d in schedule) {
        for (var z in schedule[d]) {
            var hours = (schedule[d][z].e - schedule[d][z].s)
            sum += (schedule[d][z].sp * hours);
        }
    }
    return sum / (24 * 7.0);
}

function calc_schedule_esp(sched) {
    var fixsched = JSON.parse(JSON.stringify(sched));
    for (var d in fixsched) {
        fixsched[d].s *= 100;
        fixsched[d].e *= 100;
        fixsched[d].sp *= 100;
    }
    return fixsched;
}

// function for checking if the page is visible or not
// (if not visible it will stop updating data)
function checkVisibility() {
    $(window).bind("focus", function(event) {
        visibleFlag = 1;
    });

    $(window).bind("blur", function(event) {
        visibleFlag = 0;
    });
}

function setHysteresis() {
    console.log("Setting Hystersis");

    var cmd = new Object();
    cmd.event = "set_hysteresis";
    cmd["hysteresis_h"] = parseInt(document.getElementById('thermostat1hysteresishigh').value);
    cmd["hysteresis_l"] = parseInt(document.getElementById('thermostat1hysteresislow').value);

    var json = JSON.stringify(cmd);
    connection.send(json);
    console.log(cmd);


}
function save(param, payload) {
    console.log("Saving " + param + " " + payload)
    doingsave = true;

    var cmd = new Object();
    cmd.event = param;
    if (param == "thermostat_state") {
        cmd.state = payload;
    } else if (param == "thermostat_manualsetpoint") {
        cmd.temperature = payload;
    } else if (param == "thermostat_schedule") {
        // TODO:
        var strtointdays = {
            'mon': 0,
            'tue': 1,
            'wed': 2,
            'thu': 3,
            'fri': 4,
            'sat': 5,
            'sun': 6
        };

        cmd["day"] = strtointdays[day];
        cmd["schedule"] = calc_schedule_esp(schedule[day]);
    } else if (param == "thermostat_mode") {

        cmd.mode = parseInt(payload);
    }

    var json = JSON.stringify(cmd);
    console.log(json);

    connection.send(json);
    update();

}


