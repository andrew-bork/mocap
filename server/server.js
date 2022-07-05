/* jshint esversion: 8 */
const http = require("http");
const express = require("express");
const fs = require("fs");
const net = require("net");

const app = express();
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server, {
    cors: {
        origin: "*",
        methods: ["GET", "POST"]
    }
});
const port = 80;
const config = require("../config/config.json");

const SOCKET_LOCATION = config.socket_path;

const process = require("process");

function exitHandler(options, exitCode){
    console.log(`Exiting with code "${exitCode}"`);

    server.close();
    fs.unlinkSync(SOCKET_LOCATION);

    

    if (options.cleanup) console.log('clean');
    if (exitCode || exitCode === 0) console.log(exitCode);
    if (options.exit) process.exit();
}


//do something when app is closing
process.on('exit', exitHandler.bind(null,{cleanup:true}));

//catches ctrl+c event
process.on('SIGINT', exitHandler.bind(null, {exit:true}));

// catches "kill pid" (for example: nodemon restart)
process.on('SIGUSR1', exitHandler.bind(null, {exit:true}));
process.on('SIGUSR2', exitHandler.bind(null, {exit:true}));

//catches uncaught exceptions
process.on('uncaughtException', exitHandler.bind(null, {exit:true}));

server.listen(port, () => {
    console.log("listenen");

    var lastSensorOutput = [];

    var server = net.createServer((connection) =>{
        connection.on("data", (data) => {
            lastSensorOutput = data.toString().split(" ");
            // console.log(lastSensorOutput);
            io.emit("sensor", lastSensorOutput);
        });

        connection.on("end", () => {
            console.log("Connection lost");
        })

        io.on("connection", (socket) => {
            socket.on("cmd", (cmd) => {
                cmd = `${cmd}`;
                console.log(`Sending command "${cmd}"`);
               connection.write(cmd); 
            });
        });
    });

    server.listen(SOCKET_LOCATION, () => {
        console.log(`Socket Server created at ${SOCKET_LOCATION}.`);
    });
});