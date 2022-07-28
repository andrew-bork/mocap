<script>
import ThreeCanvas from "./ThreeCanvas.svelte";

// const io = require("socket.io-client");
// const socket = io("")
import io from "socket.io-client";
let socket = io("http://rpi/");
socket.on("sensor", (data) => {
	data = data.map((k) => parseFloat(k));

	rotation.roll = data[1];
	rotation.pitch = data[2];
	rotation.yaw = data[3];
	// console.log(data);
});

let rotation = {
	roll: 0,
	pitch: 0,
	yaw: 0
};

</script>

<main>
	<ThreeCanvas rotation={rotation}/>
</main>

<style>

	* {
		box-sizing: border-box;
	}

	:global(body){
		padding: 0;
	}
</style>