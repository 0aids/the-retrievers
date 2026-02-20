import {
    Popup,
    Marker,
    Circle,
    Polyline,
    TileLayer,
    useMapEvents,
    MapContainer,
    ScaleControl,
    AttributionControl,
} from "react-leaflet";
import L, { Icon } from "leaflet";
import { useState, useEffect, useRef } from "react";
import markerIconPng from "leaflet/dist/images/marker-icon.png";

import { useAppState } from "../../hooks/StateContext";
import { DistanceControl, calculateDistance } from "./distance";
import { useWatchPosition } from "../../hooks/useWatchPosition";

export default function MapView() {
    // this will be the position for like half a sec or smth before we get actual position
    const INITIAL_POSITION: L.LatLngExpression = [-36.851939, 174.770379];

    const { state } = useAppState();
    const mapRef = useRef<L.Map | null>(null);

    const userPosition = useWatchPosition((p) => {
        mapRef.current?.flyTo([p.lat, p.lon], mapRef.current?.getZoom());
    });
    const psatPosition = {
        lat: state?.gps.latitude,
        lon: state?.gps.longitude,
    };

    const [showPath, setShowPath] = useState(true);
    const [followPsat, setFollowPsat] = useState(false);
    const [psatTrail, setPsatTrail] = useState<L.LatLngExpression[]>([]);
    const [distanceText, setDistanceText] = useState<string | undefined>(
        "Unknown",
    );
    const [psatToMe, setPsatToMe] = useState<
        [L.LatLngExpression, L.LatLngExpression]
    >([
        [0, 0],
        [0, 0],
    ]);

    useEffect(() => {
        if (psatPosition.lat && psatPosition.lon) {
            if (followPsat) {
                mapRef.current?.panTo([psatPosition.lat, psatPosition.lon], {
                    animate: true,
                    duration: 0.6,
                });
            }
            setPsatTrail((prev) => {
                const next = [
                    ...prev,
                    [psatPosition.lat, psatPosition.lon] as L.LatLngExpression,
                ];
                if (next.length > 100) next.shift();
                return next;
            });
        } else {
            // for some reason TS wont let me use a guard clause so i gotta do this fuckass else :(
            return;
        }

        if (userPosition) {
            setPsatToMe([
                [psatPosition.lat, psatPosition.lon],
                [userPosition.lat, userPosition.lon],
            ]);
            const d = calculateDistance(
                psatPosition.lat,
                psatPosition.lon,
                userPosition.lat,
                userPosition.lon,
            );
            setDistanceText(`${d.toFixed(2)}m`);
        }
    }, [psatPosition, userPosition]);

    return (
        <>
            <section>
                <div className="h-[45vh] rounded-xl border mx-1 border-gray-800 shadow-xl overflow-hidden">
                    <MapContainer
                        center={INITIAL_POSITION}
                        zoom={19}
                        ref={mapRef}
                        attributionControl={false}
                        style={{ height: "100%", width: "100%" }}
                    >
                        <TileLayer
                            url="/static/uoa_tiles/{z}/{x}/{y}.png"
                            maxNativeZoom={19}
                            maxZoom={22}
                        />
                        <ScaleControl
                            imperial={false}
                            metric
                            position="bottomleft"
                        />
                        <AttributionControl
                            position="bottomright"
                            prefix="The Retrievers"
                        />
                        <DistanceControl value={distanceText} />

                        <LocationMarker />

                        {userPosition && (
                            <Circle
                                center={[userPosition.lat, userPosition.lon]}
                                radius={3}
                                pathOptions={{
                                    color: "blue",
                                    fillColor: "#3987d9",
                                    fillOpacity: 0.5,
                                }}
                            />
                        )}

                        {psatPosition.lat && psatPosition.lon && (
                            <Circle
                                center={[psatPosition.lat, psatPosition.lon]}
                                radius={3}
                                pathOptions={{
                                    color: "red",
                                    fillColor: "#d93939",
                                    fillOpacity: 0.5,
                                }}
                            />
                        )}

                        {showPath && psatTrail.length > 0 && (
                            <Polyline
                                positions={psatTrail}
                                pathOptions={{ color: "red" }}
                            />
                        )}

                        {showPath && psatToMe && (
                            <Polyline
                                positions={psatToMe}
                                pathOptions={{
                                    color: "#0ecf45",
                                    weight: 2,
                                    opacity: 1,
                                    dashArray: "5",
                                }}
                            />
                        )}
                    </MapContainer>
                </div>
            </section>

            <section className="grid grid-cols-3 gap-2">
                <button
                    className="bg-gray-900 gs-btn border border-gray-700 py-2 rounded-xl text-sm"
                    onClick={() => {
                        if (userPosition) {
                            mapRef.current?.panTo(
                                [userPosition.lat, userPosition.lon],
                                { animate: true, duration: 0.6 },
                            );
                        }
                    }}
                >
                    Go To Me
                </button>

                <button
                    className="bg-gray-900 gs-btn border border-gray-700 py-2 rounded-xl text-sm"
                    onClick={() => {
                        if (psatPosition.lat && psatPosition.lon)
                            mapRef.current?.panTo(
                                [psatPosition.lat, psatPosition.lon],
                                { animate: true, duration: 0.6 },
                            );
                        setFollowPsat((v) => !v);
                    }}
                >
                    {followPsat ? "Unfollow PSAT" : "Follow PSAT"}
                </button>

                <button
                    className="bg-gray-900 gs-btn border border-gray-700 py-2 rounded-xl text-sm"
                    onClick={() => setShowPath((s) => !s)}
                >
                    {showPath ? "Unshow Path" : "Show Path"}
                </button>
            </section>
        </>
    );
}

function LocationMarker() {
    const [position, setPosition] = useState<L.LatLngExpression | null>(null);

    useMapEvents({
        click(e) {
            setPosition(e.latlng);
        },
        move() {
            setPosition(null);
        },
    });

    return position === null ? null : (
        <Marker
            position={position}
            icon={
                new Icon({
                    iconUrl: markerIconPng,
                    iconSize: [25, 41],
                    iconAnchor: [12, 41],
                })
            }
        >
            <Popup>{position.toString()}</Popup>
        </Marker>
    );
}
