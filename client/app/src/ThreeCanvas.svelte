<script>
    import { Scene, BoxGeometry,  MeshStandardMaterial, Mesh, WebGLRenderer, PerspectiveCamera, DirectionalLight, Vector3, Euler } from 'three';
    import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';

    export let rotation;
    
    const scene = new Scene();
    const camera = new PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);

    const renderer = new WebGLRenderer();
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);

    const geometry = new BoxGeometry(1, 1, 1);
    const material = new MeshStandardMaterial({color: 0x00FF00 });
    const cube = new Mesh(geometry, material);

    const sun = new DirectionalLight(0xFFFFFF, 0.8);
    sun.position.set(2.0,2.0,3.0);
    const back = new DirectionalLight(0x5599FF, 0.2);
    back.position.set(-1.0, -1.0, -1.0);

    sun.lookAt(0,0,0);
    back.lookAt(0,0,0);

    scene.add(sun);
    scene.add(cube);
    scene.add(back);

    camera.position.z = 5;

    const controls = new OrbitControls(camera, renderer.domElement);

    controls.enableDamping = true;

    $: cube.setRotationFromEuler(new Euler(rotation.pitch, rotation.yaw, rotation.roll, "YXZ"));

    function animate() {
        controls.update();
        requestAnimationFrame(animate);
        renderer.render(scene, camera);
    }
    animate();
</script>
