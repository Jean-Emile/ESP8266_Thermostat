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

    // Get the Relay checkbox
    var checkBox = document.getElementById("relay");

    // If the checkbox is checked, display the output text
    var cmd = '#relay=';
    if (checkBox.checked == true){
        cmd= cmd+"1";
    } else {
        cmd= cmd+"0";
    }

    console.log('Relay: ' + cmd);
    connection.send(cmd);
}


function restartBoard() {
    var r =  confirm('Are you sure you want to restart?')
    if (r == true) {
        var cmd = '#restart=1';
        console.log(cmd);
        connection.send(cmd);
    }

}