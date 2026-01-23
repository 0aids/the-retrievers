let lat: number | undefined;
let long: number | undefined;
let psatLat: number | undefined;
let psatLong: number | undefined;
let circle: L.Circle | undefined;
let psatCircle: L.Circle | undefined;

const map = L.map("map").setView([-36.851939, 174.770379], 19);

L.tileLayer("/static/assets/tiles1/{z}/{x}/{y}.png", {
    maxZoom: 19,
}).addTo(map);

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

setInterval(() => {
    (async () => {
        console.log("Requesting State...");
        const data = await (
            await fetch("/api/state", { method: "GET" })
        ).json();

        psatLat = data.gps.latitude;
        psatLong = data.gps.longitude;

        if (psatLat === undefined || psatLong === undefined) {
            return;
        }

        if (!psatCircle) {
            psatCircle = L.circle([psatLat, psatLong], {
                color: "red",
                fillColor: "#d93939",
                fillOpacity: 0.5,
                radius: 3,
            }).addTo(map);
        } else {
            psatCircle.setLatLng([psatLat, psatLong]);
        }

        console.log(`Recieved PSAT Coords: ${psatLat}, ${psatLong}`);
    })();
}, 500);
