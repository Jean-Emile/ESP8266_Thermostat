

function RelayControl() {
    var checkBox = document.getElementById("relay");
    var cmd = new Object();
    cmd.event = "relay";
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
        cmd.event = "restart";
        connection.send(JSON.stringify(cmd));
    }

}