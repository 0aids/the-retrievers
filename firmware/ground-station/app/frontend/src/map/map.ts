import * as L from "leaflet";

export const initialPosition: L.LatLngExpression = [-36.851939, 174.770379];

export const map = L.map("map", { attributionControl: false }).setView(
    initialPosition,
    19,
);

const popup = L.popup();

L.tileLayer("/static/uoa_tiles/{z}/{x}/{y}.png", {
    maxZoom: 19,
}).addTo(map);

map.on("click", (e: L.LeafletMouseEvent) => {
    popup.setLatLng(e.latlng).setContent(e.latlng.toString()).openOn(map);
});

L.control
    .scale({
        metric: true,
        imperial: false,
        position: "bottomleft",
    })
    .addTo(map);

L.control
    .attribution({
        prefix: "The Retrievers",
        position: "bottomright",
    })
    .addTo(map);
