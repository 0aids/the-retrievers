import * as L from "leaflet";
import { map } from "./map";

let circle: L.Circle | undefined;
export let userPos: { lat: number; lon: number } | undefined;

navigator.geolocation.getCurrentPosition((pos) => {
    userPos = {
        lat: pos.coords.latitude,
        lon: pos.coords.longitude,
    };

    circle = L.circle([userPos.lat, userPos.lon], {
        color: "blue",
        fillColor: "#3987d9",
        fillOpacity: 0.5,
        radius: 3,
    }).addTo(map);
});

navigator.geolocation.watchPosition(
    (pos) => {
        userPos = {
            lat: pos.coords.latitude,
            lon: pos.coords.longitude,
        };
        circle?.setLatLng([userPos.lat, userPos.lon]);
    },
    undefined,
    { enableHighAccuracy: true, maximumAge: 0 },
);

export function panToMe() {
    if (userPos) {
        map.panTo([userPos.lat, userPos.lon] as L.LatLngExpression, {
            animate: true,
            duration: 0.6,
        });
    }
}
