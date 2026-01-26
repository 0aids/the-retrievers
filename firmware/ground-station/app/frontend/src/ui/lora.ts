const lastPacketEl = document.getElementById("lora-last-packet");
const packetsRecievedEl = document.getElementById("lora-packets-rx");

export function updateLoraStatus(
    loraPacket: number | null,
    packetsRecieved: number | null,
) {
    if (!lastPacketEl || !packetsRecievedEl) {
        return;
    }
    lastPacketEl.textContent = `${loraPacket}`;
    packetsRecievedEl.textContent = `${packetsRecieved}`;
}
