# Tabi-chan
>Our plan for Third Space Hack Club - Week 3!

A travel companion that tracks and logs your journey, and provides the weather data, and geo data.

## Project Overview
An adorable little travel companion that lives in a small device that has an e-ink display and a GPS sensor.

Tabi-chan (旅-chan) is a little gps sensor that you can clip onto yourself when out on a journey. She then sends over the tracked data to your phone via BLE and dislays it all on a web app. The basic features of the web app are route tracking, location overview, and weather overview. 

### <ins>Key Features of Box</ins> 
<ul style="margin-top: -5px; margin-bottom: 0; padding-left: 10px">
    <p style="margin-bottom: 5px">
        <b>GPS Tracking & Activity Logging</b>
    <ul style="padding-left: 45px">
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>GPS:</b> Calculate latitude, longitude and altitude.</p>
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>Route tracking:</b> Route tracking using Leaflet, calculates pace and distance</p>
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>Location Overview:</b> Query OpenStreetMap API to get current location.</p>
        <p style="text-indent: -24px; margin-bottom: 5px">
            <b>Weather Overview:</b> Uses Open-Meteo API to fetch weather info.</p>


### Say hi to Tabi-chan
<img width="814" height="696" alt="image" src="https://github.com/user-attachments/assets/f7d7c555-74e8-477e-823f-21f3d0015ca0" />

<img width="779" height="711" alt="image" src="https://github.com/user-attachments/assets/78891822-f6b6-4e4a-b04c-4b23d0123aba" />

## PCB
| Top | Bottom |
| --- | --- |
| <img width="740" height="333" alt="image" src="https://github.com/user-attachments/assets/7d52e453-8497-4642-b727-67bf9fedc79b" /> | <img width="743" height="322" alt="image" src="https://github.com/user-attachments/assets/537cf7fb-8611-47cb-b3b1-4a3ad4f466ad" /> |

 <img width="620" height="340" alt="image" src="https://github.com/user-attachments/assets/fbd7ca31-2a94-4cf3-b520-4a813a8a6299" />

## Case

<img width="500" alt="image" src="https://github.com/user-attachments/assets/2f86b315-3521-44e1-9a11-d64529026024" />

<img width="500" alt="image" src="https://github.com/user-attachments/assets/b8b68640-d955-4016-ac32-fe97c7fc25fe" />

<img width="500" alt="image" src="https://github.com/user-attachments/assets/0e8aff74-76ca-45e9-9384-bb75a7e996ec" />

<img width="500" alt="image" src="https://github.com/user-attachments/assets/eca0782a-b5bd-4522-9717-1eb9e67ca52a" />

## BOM
| Reference | Value | Qty | Price | Source |
| :--- | :--- | :--- | :--- | :--- |
| U6 | IP5306-I2C | 1 | $0.35 | https://www.lcsc.com/product-detail/New-Arrivals_INJOINIC-IP5306-I2C_C488349.html |
| U5 | AP2112K-3.3TRG1 | 1 | $0.17 | https://jlcpcb.com/partdetail/AP2112K-3.3TRG1/C51118 |
| U2 | NEO-6M-0-001_C32413118 | 1 | $6.69 | https://jlcpcb.com/partdetail/UBLOX-NEO_6M_0001/C32413118 |
| U1 | ESP32-S3-WROOM-1(N16R8) | 1 | $3.79 | https://www.lcsc.com/product-detail/C2913202.html |
| SW1 | SW_Push | 1 | $0.02 | https://www.lcsc.com/product-detail/C318884.html |
| R12 | 1k | 1 | $0.0033 | https://www.lcsc.com/product-detail/C5362357.html |
| R8 | 2R | 1 | $0.21 | https://www.digikey.com/en/products/detail/vishay-dale/RCS08052R00FKEA/5868970 |
| R4,R5,R6,R7,R9,R10,R11 | 10k | 7 | $0.0029 | https://www.lcsc.com/product-detail/chip-resistor-surface-mount_tyohm-rmc-0603-10k-j-n_C5362358.html |
| R3 | 0.5R | 1 | $0.0013 | https://jlcpcb.com/partdetail/VO-SCR0603JR500/C3017643 |
| R1,R2 | 5.1k | 2 | $0.0016 | https://www.lcsc.com/product-detail/C23186.html |
| Q1,Q2 | BSS138BKW | 2 | RM1.10 | https://www.digikey.my/en/products/detail/nexperia-usa-inc/BSS138BKW-115/2779824 |
| L1 | 1uH | 1 | $0.0742 | https://jlcpcb.com/partdetail/YJYCOIN-YHNR30121R0M/C20416185 |
| J4 | Conn_01x08 | 1 | $0.0491 | https://www.lcsc.com/product-detail/C492407.html |
| J3 | Conn_01x02 | 1 | $0.0180 | https://www.lcsc.com/product-detail/C492401.html |
| J2 | U.FL | 1 | $0.1072 | https://www.lcsc.com/product-detail/C88374.html |
| J1 | USB_C_Receptacle_USB2.0_16P | 1 | $0.0415 | https://www.lcsc.com/product-detail/C2765186.html |
| C11,C12,C13 | 0.1uF | 3 | $0.0107 | https://www.lcsc.com/product-detail/C131394.html |
C8,C9,C10,C14 | 1uF | 4 | $0.1137 | https://jlcpcb.com/partdetail/KEMET-C0603C105K3RACTU/C2177372|
C6,C7,C15 | 10uF | 3 | $0.1027 | https://jlcpcb.com/partdetail/AIDECAPACITOR-0805X5R106M500NT/C49326683|
C1,C2,C3,C4,C5 | 22uF | 5 | $0.0844 | https://jlcpcb.com/partdetail/CCTC-TCC0805X5R226K160FT/C48885730|
