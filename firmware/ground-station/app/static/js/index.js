let lat, long;

const map = L.map("map").setView([-36.851939, 174.770379], 19);

L.tileLayer("/static/assets/tiles1/{z}/{x}/{y}.png", {
    maxZoom: 19,
}).addTo(map);

navigator.permissions
    .query({ name: "geolocation" })

const popup = L.popup();
map.on("click", (e) => {
    popup
        .setLatLng(e.latlng)
        .setContent(e.latlng.toString())
        .openOn(map)
});


let circle;
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

        if (circle)
            circle.setLatLng(L.latLng(lat, long));
    },
    null,
    { enableHighAccuracy: true, maximumAge: 0 },
);