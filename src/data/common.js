var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
connection.onopen = function () {
    connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {  
    console.log('Server: ', e.data);
};
connection.onclose = function(){
    console.log('WebSocket connection closed');
};


function RelayControl() {
    var checkBox = document.getElementById("relay");
    var cmd = new Object();
    cmd.cmd = "relay";
    if (checkBox.checked == true){
        cmd.state = true;
    } else {
        cmd.state = false;
    }
    var json = JSON.stringify(cmd);
    console.log(json);
    connection.send(json);
}

function restartBoard() {
    var r =  confirm('Are you sure you want to restart?')
    if (r == true) {
        var cmd = new Object();
        cmd.cmd = "restart";
        connection.send(JSON.stringify(cmd));
    }

}