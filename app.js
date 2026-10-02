const SERVICE_UUID = '';
const CHAR_UUID    = '';

let bleDevice = null;
let trackPoints = []; //stores all data

async function connectBLE() {
    try {
        bleDevice = await navigator.bluetooth.requestDevice({
            filters: [{ services: [SERVICE_UUID] }]
        });

        const server = await bleDevice.gatt.connect();
        const service = await server.getPrimaryService(SERVICE_UUID);
        const characteristic = await service.getCharacteristic(CHAR_UUID);

        await characteristic.startNotifications();
        characteristic.addEventListener('characteristicvaluechanged', handleGpsPacket);
    } catch (error) {
        console.error('Error connecting to BLE device:', error);
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


let totalDistanceMeters = 0;

function haversineDistance(lat1, lon1, lat2, lon2) {
    const R = 6371000; // Radius of the Earth in meters
    const toRadians = (degrees) => degrees * (Math.PI / 180);

    const dLat = toRadians(lat2 - lat1);
    const dLon = toRadians(lon2 - lon1);

    const a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
              Math.cos(toRadians(lat1)) * Math.cos(toRadians(lat2)) *
              Math.sin(dLon / 2) * Math.sin(dLon / 2);

    return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
}

function recordPoint(point) {
    if (trackPoints.length > 0) {
        const lastPoint = trackPoints[trackPoints.length - 1];
        const distance = haversineDistance(lastPoint.lat, lastPoint.lon, point.lat, point.lon);

        if (distance > 0.5) {
            totalDistanceMeters += distance;
        }
    }

    trackPoints.push(point);
    updateDashboard(point);
}

function updateDashboard(current) {
    const totalDistanceKm = (totalDistanceMeters / 1000).toFixed(2);
    const currentSpeedKmh = (current.speed * 3.6).toFixed(1); // Convert m/s to km/h

    let paceStr = "--:--";
    if (current.speed > 0.3) {
        const paceSecondsPerKm = 1000 / current.speed;
        const minutes = Math.floor(paceSecondsPerKm / 60);
        const seconds = Math.floor(paceSecondsPerKm % 60).toString().padStart(2, '0');
        paceStr = `${minutes}:${seconds}`;
    }

    document.getElementById('totalDistance').textContent = `${totalDistanceKm} km`;
    document.getElementById('currentSpeed').textContent = `${currentSpeedKmh} km/h`;
    document.getElementById('currentPace').textContent = `${paceStr} /km`;
}
