# Tabi-chan ☀️🧭
>Our plan for Third Space Hack Club - Week 3!

A travel companion that tracks and logs your journey, and provides the weather data, and geo data.

## 💠 Project Overview 💠
An adorable little travel companion that lives in a small device that has a e-ink display and a GPS sensor.

Tabi-chan (旅-chan, the little traveler) connects embedded hardware with modern web tech. Equipped with an onboard GPS receiver, battery monitoring, and an E-Ink screen showing Tabi-chan's facial expressions and status, it streams raw positioning data directly over BLE to a companion Progressive Web App (PWA) with live Leaflet mapping, GPX exports, and local weather lookup—no native app installs or proprietary cloud subscriptions required.

### <ins>Key Features of Box</ins> 
<ul style="margin-top: -5px; margin-bottom: 0; padding-left: 10px">
    <p style="margin-bottom: 5px">
        <b>GPS Tracking & Activity Logging</b>
    <ul style="padding-left: 45px">
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>GNSS Receiver & Live Telemetry:</b> An internal GPS receiver (NEO-6M) reads satellite signals to calculate latitude, longitude and altitude.</p>
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>Real-Time Breadcrumb Polyline:</b> Active routes are dynamically plotted onto a Leaflet.js map layer with auto-panning breadcrumb markers, pace calculation, and live elapsed moving timers.</p>
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>Location Overview:</b> Queries OpenStreetMap's Nominatim API to resolve raw GPS coordinates into human-readable locations.</p>
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>Weather Overview:</b> Uses Open-Meteo API to fetch weather info.</p>
            

        
