const SERVICE_UUID = '';
const CHAR_UUID    = '';

const  MIN_SPEED_THRESHOLD = 0.3; // Minimum speed in m/s to consider for pace calculation
const MIN_DISTANCE_THRESHOLD = 1.5; // Minimum distance in meters to consider for distance calculation

const TrackerState = Object.freeze({
    IDLE: 'idle',
    TRACKING: 'tracking',
    PAUSED: 'paused'
});

let currentState = TrackerState.IDLE;
let bleDevice = null;
let gattCharacteristic = null;

//Recording data
let trackPoints = []; 
let totalDistanceMeters = 0;
let movingTimeSeconds = 0;
let lastTimerUpdate = null;
let timerInterval = null;

//leaflet map references
let map = null;
let routePolyline = null;
let currentPositionMarker = null;
let isMapAutoPanEnabled = true;

//Ui event hooks
const TrackerEvents = {
    onConnectionChange: (status) => {},
    onStateChange: (state) => {},
    onGpsUpdate: (point) => {},
    onDashboardUpdate: (stats) => {},
    onTimerUpdate: (formattedTime, totalSeconds) => {}
};

//leaflet map initialisation

function initMap(containerId = 'map', initialCoords = [0, 0], initialZoom = 16) {
    if (typeof L === 'undefined') {
        console.error('Leaflet library is not loaded. Please include Leaflet.js in your HTML.');
        return;
    }

    map = L.map(containerId).setView(initialCoords, initialZoom);

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);

    routePolyline = L.polyline([], {
        color: '#6600ff',
        weight: 5,
        opacity: 0.7,
        smoothFactor: 1
    })

    const markerIcon = L.divIcon({
        className: 'current-position-marker',
        html: `<div style="
        width: 20px;
        height: 20px;
        background-color: #ff0000;
        border-radius: 50%;
        box-shadow: 0 0 5px rgba(0, 0, 0, 0.5);
        "></div>`,
        iconSize: [20, 20],
        iconAnchor: [10, 10]
    });

    currentPositionMarker = L.marker(initialCoords, { icon: markerIcon }).addTo(map);

    map.on('dragstart', () => {
        isMapAutoPanEnabled = false;
    });
}

function updateMapPosition(point) {
    if (!map) return;

    const latLng = [point.lat, point.lon];

    if (currentPositionMarker) {
        currentPositionMarker.setLatLng(latLng);
    }

    if (currentState === TrackerState.TRACKING && routePolyline) {
        routePolyline.addLatLng(latLng);
    }

    if (isMapAutoPanEnabled) {
        map.panTo(latLng, { animate: true, duration: 0.5 });
    }
}

function enableMapAutoPan() {
    isMapAutoPanEnabled = true;
    
    if(currentPositionMarker && map) {
        map.setView(currentPositionMarker.getLatLng(), map.getZoom());
    }
}

function resetMapTrack() {
    if (routePolyline) {
        routePolyline.setLatLngs([]);
    }
}


//ble connection
async function connectBLE() {
    try {
        bleDevice = await navigator.bluetooth.requestDevice({
            filters: [{ services: [SERVICE_UUID] }]
        });

        bleDevice.addEventListener('gattserverdisconnected', onDisconnected);

        const server = await bleDevice.gatt.connect();
        const service = await server.getPrimaryService(SERVICE_UUID);
        gattCharacteristic = await service.getCharacteristic(CHAR_UUID);

        await gattCharacteristic.startNotifications();
        gattCharacteristic.addEventListener('characteristicvaluechanged', handleGpsPacket);
        
    } catch (error) {
        console.error('Error connecting to BLE device:', error);
    }
}

function disconnectBLE() {
    if (bleDevice && bleDevice.gatt.connected) {
        bleDevice.gatt.disconnect();
    }
}

function onDisconnected() {
    console.warn('Device disconnected');

    if (currentState === TrackerState.TRACKING) {
        pauseTracking();
    }
}

function handleGpsPacket(event) {
    const dataView = event.target.value;

    if (dataView.byteLength < 12) {
        console.warn('Received packet is too short:', dataView.byteLength);
        return;
    }

    const rawLat = dataView.getInt32(0, true);
    const rawLon = dataView.getInt32(4, true);
    const elevation = dataView.getInt16(8, true);
    const rawSpeed = dataView.getUint16(10, true);
    
    const lat = rawLat / 1e7;
    const lon = rawLon / 1e7;
    const speed = rawSpeed / 100; // Convert cm/s to m/s
    const timestamp = new Date().toISOString();

    if (lat === 0 && lon === 0) return;

    const point = { lat, lon, elevation, speed, time: timestamp };

    updateMapPosition(point);

    TrackerEvents.onGpsUpdate(point);

    if (currentState === TrackerState.TRACKING) {
        recordPoint(point);
    }
}



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

    if (point.lat === 0 && point.lon === 0) return; // Ignore invalid points

    if (trackPoints.length > 0) {
        const lastPoint = trackPoints[trackPoints.length - 1];
        const distance = haversineDistance(lastPoint.lat, lastPoint.lon, point.lat, point.lon);

        if (distance >= MIN_DISTANCE_THRESHOLD) {
            totalDistanceMeters += distance;
            trackPoints.push(point);
        }
    } else {
        trackPoints.push(point);
    }

    emitDashboardStats(point);
}

function emitDashboardStats(current) {
    const totalDistanceKm = (totalDistanceMeters / 1000).toFixed(2);
    const currentSpeedKmh = (current.speed * 3.6).toFixed(1); // Convert m/s to km/h

    let paceStr = "--:--";
    if (current.speed > 0.3) {
        const paceSecondsPerKm = 1000 / current.speed;
        const minutes = Math.floor(paceSecondsPerKm / 60);
        const seconds = Math.floor(paceSecondsPerKm % 60).toString().padStart(2, '0');
        paceStr = `${minutes}:${seconds}`;
    }

    TrackerEvents.onDashboardUpdate({
        totalDistanceKm,
        currentSpeedKmh,
        paceStr,
        elevationM: current.elevation
    });
}

function startTracking() {
    if (currentState === TrackerState.TRACKING) {
        trackPoints = [];
        totalDistanceMeters = 0;
        movingTimeSeconds = 0;
        resetMapTrack();
    }
    
    currentState = TrackerState.TRACKING;
    lastTimerUpdate = Date.now();

    timerInterval = setInterval(() => {
        const now = Date.now();
        const delta = Math.floor((now - lastTimerUpdate) / 1000);
        if (delta >= 1) {
            movingTimeSeconds += delta;
            lastTimerUpdate = now;

            const hours = Math.floor(movingTimeSeconds / 3600);
            const minutes = Math.floor((movingTimeSeconds % 3600) / 60);
            const seconds = movingTimeSeconds % 60;

            const formattedTime = hours > 0
                ? `${hours}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`
                : `${minutes}:${seconds.toString().padStart(2, '0')}`;
            
            TrackerEvents.onTimerUpdate(formattedTime, movingTimeSeconds);
        }
    }, 1000);

    TrackerEvents.onStateChange(currentState);
}

function pauseTracking() {
    currentState = TrackerState.PAUSED;
    clearInterval(timerInterval);
    timerInterval = null;
    TrackerEvents.onStateChange(currentState);
}

function stopTracking() {
    pauseTracking();
    currentState = TrackerState.IDLE;
    TrackerEvents.onStateChange(currentState);

    if (routePolyline && trackPoints.length > 1) {
        map.fitBounds(routePolyline.getBounds(), { padding: [30, 30] });
    }
}
