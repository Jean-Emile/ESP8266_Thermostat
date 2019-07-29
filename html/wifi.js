
var wb_url = 'ws://esp8266.local:81/'
var connection = new WebSocket(wb_url);
connection.onopen = function () {
    console.log("connected...");
    updateclock();
    setStatus("Connected",5,0);

};

connection.onopen = function () {
    var cmd = new Object();
    cmd.event = "do_wifi_scan";
    var json = JSON.stringify(cmd);
    connection.send(json);

};

connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};

connection.onmessage = function (e) {  
    console.log('Server: ', e.data);

    var json = JSON.parse(e.data);

    if( json["event"] == "wifi_scan"){
        var networks = json["networks"];
        var aps = document.getElementById("aps");
        while(aps.firstChild){
            aps.removeChild(aps.firstChild);
        }
        for(var i = 0; i < networks.length; i++) {
            var network = networks[i];

            var div=document.createElement("div");
            div.id="apdiv";
            var rssi=document.createElement("div");
            var rssiVal= -Math.floor(network["rssi"]/51)*32;
            rssi.className="icon";
            rssi.style.backgroundPosition="0px "+rssiVal+"px";
            var encrypt=document.createElement("div");
            var encVal="-64"; //assume wpa/wpa2
            if (network["encryption"] == 0) encVal="0"; //open
            if (network["encryption"] == 1) encVal="-32"; //wep
            encrypt.className="icon";
            encrypt.style.backgroundPosition="-32px "+encVal+"px";
            var input=document.createElement("input");
            input.type="radio";
            input.name="essid";
            input.value=network["ssid"];
            // if (currAp==ap.essid) input.checked="1";
            input.id=network["ssid"];
            var label=document.createElement("label");
            label.htmlFor="opt-"+network["ssid"];
            label.textContent=network["ssid"];
            div.appendChild(input);
            div.appendChild(rssi);
            div.appendChild(encrypt);
            div.appendChild(label);

            aps.appendChild(div);
        }
    }



};
function getSelectedEssid() {
    var e=document.forms.wifiform.elements;
    for (var i=0; i<e.length; i++) {
        if (e[i].type=="radio" && e[i].checked) return e[i].value;
    }
    return currAp;
}



connection.onclose = function(){
    console.log('WebSocket connection closed');
};



function saveWifiConf()
{
    var ssid =  getSelectedEssid();
    var password = document.getElementById("pass");
    // TODO send

}