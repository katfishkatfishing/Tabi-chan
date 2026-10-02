const SERVICE_UUID = '';
const CHAR_UUID    = '';

let bleDevice = null;
let trackPoints = []; //stores all data

async function connectBLE() {
    try {
        
    }
}

function handleGpsPacket(event) {
    const dataView = event.target.value;

    const rawLat = dataView.getFloat32(0, true);
    const rawLon = dataView.getFloat32(4, true);
    const elevation = dataView.getInt16(8, true);
    const speed = dataView.getUint16(10, true);
    
    const lat = rawLat / 1e7;
    const lon = rawLon / 1e7;
    const timestamp = new Date().toISOString();

    recordPoint({ lat, lon, elevation, speed: speed / 100, time: timestamp });
}

