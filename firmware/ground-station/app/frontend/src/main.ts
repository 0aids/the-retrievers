import "./main.css";
import * as L from "leaflet";

let lat: number | undefined;
let long: number | undefined;
let psatLat: number | undefined;
let psatLong: number | undefined;
let circle: L.Circle | undefined;
let psatCircle: L.Circle | undefined;
let showPath = true;
let followPsat = false;

const trail: L.LatLngExpression[] = [];

const map = L.map("map").setView([-36.851939, 174.770379], 19);

const TextBox = L.Control.extend({
    onAdd: function () {
        var text = L.DomUtil.create("div");
        text.innerHTML =
            '<div class="bg-gray-700 text-white rounded-xl px-2 py-1 text-center w-30"><p></p></div>';
        return text;
    },
    updateText: function (text: string) {
        const container = (this as any as L.Control).getContainer();
        if (!container) return;
        container.getElementsByTagName("p")[0].innerHTML = text;
    },
});

const textbox = new TextBox({ position: "topright" });
if (map) {
    textbox.addTo(map);
}

L.tileLayer("/static/assets/tiles1/{z}/{x}/{y}.png", {
    maxZoom: 19,
}).addTo(map);
const polyline = L.polyline(trail, { color: "red" }).addTo(map);

const button = document.getElementById("toggle-path");
button?.addEventListener("click", () => {
    console.log(`Show Path ${showPath}`);
    showPath = !showPath;

    if (!showPath) {
        polyline.remove();
    } else {
        polyline.addTo(map);
    }
});

const button2 = document.getElementById("follow-me");
button2?.addEventListener("click", () => {
    map.panTo([lat, long] as L.LatLngExpression);
});

const button3 = document.getElementById("follow-psat");
button3?.addEventListener("click", () => {
    console.log(`Follow PsSAT ${followPsat}`);
    followPsat = !followPsat;
});

navigator.permissions.query({ name: "geolocation" as PermissionName });

const popup = L.popup();
map.on("click", (e: L.LeafletMouseEvent) => {
    popup.setLatLng(e.latlng).setContent(e.latlng.toString()).openOn(map);
});

navigator.geolocation.getCurrentPosition(
    (position) => {
        lat = position.coords.latitude;
        long = position.coords.longitude;

        circle = L.circle([lat, long], {
            color: "blue",
            fillColor: "#3987d9",
            fillOpacity: 0.5,
            radius: 3,
        }).addTo(map);
    },
    (e) => alert(e.message),
);

navigator.geolocation.watchPosition(
    (position) => {
        lat = position.coords.latitude;
        long = position.coords.longitude;

        if (circle && lat !== undefined && long !== undefined) {
            circle.setLatLng([lat, long]);
        }
    },
    () => {},
    { enableHighAccuracy: true, maximumAge: 0 },
);

const calculateDistance = (
    lat1: number,
    lon1: number,
    lat2: number,
    lon2: number,
) => {
    // stolen from https://www.movable-type.co.uk/scripts/latlong.html

    const R = 6371e3; // metres
    const φ1 = (lat1 * Math.PI) / 180; // φ, λ in radians
    const φ2 = (lat2 * Math.PI) / 180;
    const Δφ = ((lat2 - lat1) * Math.PI) / 180;
    const Δλ = ((lon2 - lon1) * Math.PI) / 180;

    const a =
        Math.sin(Δφ / 2) * Math.sin(Δφ / 2) +
        Math.cos(φ1) * Math.cos(φ2) * Math.sin(Δλ / 2) * Math.sin(Δλ / 2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));

    const d = R * c; // in metres

    return d;
};

setInterval(() => {
    (async () => {
        console.log("Requesting State...");
        const data = await (
            await fetch("/api/state", { method: "GET" })
        ).json();

        if (data.gps.latitude === null || data.gps.longitude === null) {
            return;
        }

        if (
            calculateDistance(
                psatLat || data.gps.latitude,
                psatLong || data.gps.longitude,
                data.gps.latitude,
                data.gps.longitude,
            ) < 1 &&
            psatCircle
        ) {
            return;
        }

        psatLat = data.gps.latitude as number;
        psatLong = data.gps.longitude as number;

        const latlong: L.LatLngExpression = [psatLat, psatLong];
        if (!psatCircle) {
            psatCircle = L.circle(latlong, {
                color: "red",
                fillColor: "#d93939",
                fillOpacity: 0.5,
                radius: 3,
            }).addTo(map);
        } else {
            psatCircle.setLatLng(latlong);
        }

        trail.push(latlong);
        polyline.setLatLngs(trail);

        if (lat && long) {
            textbox.updateText(
                `Distance to PSAT: ${calculateDistance(lat, long, psatLat, psatLong).toFixed(2)}m`,
            );
        }

        if (followPsat) {
            map.panTo(latlong);
        }

        if (trail.length > 25) {
            trail.shift();
        }

        console.log(`Recieved PSAT Coords: ${psatLat}, ${psatLong}`);
    })();
}, 500);
