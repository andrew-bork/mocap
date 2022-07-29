<script>
    import { Scene, BoxGeometry,  MeshStandardMaterial, Mesh, WebGLRenderer, PerspectiveCamera, DirectionalLight, Vector3, Euler } from 'three';
    import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
    import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader.js';


    export let rotation;
    
    const scene = new Scene();
    const camera = new PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);

    const renderer = new WebGLRenderer();
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);

    const loader = new GLTFLoader();
    let kunai;
    loader.load("models/damaged kunai.glb", (gltf) => {
        // scene.add(gltf.scene);
        kunai = gltf.scene;
        scene.add(kunai);
    })

    const sun = new DirectionalLight(0xFFFFFF, 0.8);
    sun.position.set(2.0,2.0,3.0);
    const back = new DirectionalLight(0x5599FF, 0.2);
    back.position.set(-1.0, -1.0, -1.0);

    sun.lookAt(0,0,0);
    back.lookAt(0,0,0);

    scene.add(sun);
    scene.add(back);

    camera.position.z = 5;

    const controls = new OrbitControls(camera, renderer.domElement);

    controls.enableDamping = true;

    $: if(kunai) {
        kunai.setRotationFromEuler(new Euler(rotation.pitch, rotation.yaw, rotation.roll, "YXZ"));
    }

    function animate() {
        controls.update();
        requestAnimationFrame(animate);
        renderer.render(scene, camera);
    }
    animate();
</script>
