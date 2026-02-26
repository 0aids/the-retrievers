export function capitalizeFirstLetter(val: string): string {
    return String(val).charAt(0).toUpperCase() + String(val).slice(1);
} // lowkey stole this from stackoverflow

const tasks = [
    true, // ldr
    true, // gps
    false,
    false,
    true, // battery
    false,
    false,
    false,
    false,
    false,
    false,
    false,
]; // TODO: make this probably not hardcoded
export function isTasked(id: number): boolean {
    return tasks[id];
}
