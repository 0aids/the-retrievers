from kikit import panelize_ui_impl as ki
from kikit.units import mm, deg
from kikit.panelize import Panel, BasicGridPosition, Origin
from pcbnewTransition.pcbnew import LoadBoard, VECTOR2I
from pcbnewTransition import pcbnew
from itertools import chain

from os import path

kicad_dir = path.join(
    path.dirname(__file__),
    "..",
)


############### Custom config
board1_path = path.join(kicad_dir, "./sensor-board/PSAT - Sensor PCB.kicad_pcb")
board2_path = path.join(kicad_dir, "./PSU/PSU.kicad_pcb")
board3_path = path.join(kicad_dir, "./mcu-board/mcu-board.kicad_pcb")

output_path = path.join(kicad_dir, "./panelised/panelised.kicad_pcb")


board_spacing = 3 * mm

################ KiKit Panel Config (Only deviations from default)

framing = {
    "type": "railstb",  # only rail on top and bottom
    "vspace": "3mm",  # space between board and rail
    "width": "6mm",  # Width of the rail
}

cuts = {"type": "mousebites"}
tabs = {  # Add tabs between board and board as well as board and rail
    "type": "spacing",  # Place them with constant width and spacing
    "vwidth": "5mm",
    "spacing": "5mm",
}
tooling = {"type": "3hole", "hoffset": "5mm", "voffset": "3mm", "size": "3mm"}

# Obtain full config by combining above with default
preset = ki.obtainPreset([], tabs=tabs, cuts=cuts, framing=framing, tooling=tooling)


################ Adjusted `panelize_ui#doPanelization`

# Prepare
board1 = LoadBoard(board1_path)
board2 = LoadBoard(board2_path)
board3 = LoadBoard(board3_path)
panel = Panel(output_path)


panel.inheritDesignSettings(board1)
panel.inheritProperties(board1)
panel.inheritTitleBlock(board1)


###### Manually build layout. Inspired by `panelize_ui_impl#buildLayout`
sourceArea1 = ki.readSourceArea(preset["source"], board1)
sourceArea2 = ki.readSourceArea(preset["source"], board2)
sourceArea3 = ki.readSourceArea(preset["source"], board3)

substrateCount = len(panel.substrates)  # Store number of previous boards (probably 0)
# Prepare renaming nets and references

# Actually place the individual boards
# Use existing grid positioner
# Place two boards above each other
panelOrigin = VECTOR2I(0, 0)
placer = BasicGridPosition(board_spacing, board_spacing)  # HorSpace, VerSpace
area1 = panel.appendBoard(
    board1_path,
    panelOrigin + placer.position(0, 0, None),
    origin=Origin.Center,
    sourceArea=sourceArea1,
)
area2 = panel.appendBoard(
    board2_path,
    panelOrigin + placer.position(1, 0, area1),
    origin=Origin.Center,
    sourceArea=sourceArea2,
    inheritDrc=False,
)
area3 = panel.appendBoard(
    board3_path,
    panelOrigin + placer.position(0.5, 1, area2),
    origin=Origin.Center,
    sourceArea=sourceArea3,
    inheritDrc=False,
)


substrates = panel.substrates[substrateCount:]  # Collect set of newly added boards

# Prepare frame and partition
framingSubstrates = ki.dummyFramingSubstrate(substrates, preset)
panel.buildPartitionLineFromBB(framingSubstrates)
backboneCuts = ki.buildBackBone(preset["layout"], panel, substrates, preset)


######## --------------------- Continue doPanelization

tabCuts = ki.buildTabs(preset, panel, substrates, framingSubstrates)

frameCuts = ki.buildFraming(preset, panel)


ki.buildTooling(preset, panel)
ki.buildFiducials(preset, panel)
for textSection in ["text", "text2", "text3", "text4"]:
    ki.buildText(preset[textSection], panel)
ki.buildPostprocessing(preset["post"], panel)

ki.makeTabCuts(preset, panel, tabCuts)
ki.makeOtherCuts(preset, panel, chain(backboneCuts, frameCuts))


ki.buildCopperfill(preset["copperfill"], panel)

ki.setStackup(preset["source"], panel)
ki.setPageSize(preset["page"], panel, board1)
ki.positionPanel(preset["page"], panel)

ki.runUserScript(preset["post"], panel)

ki.buildDebugAnnotation(preset["debug"], panel)

panel.save(
    reconstructArcs=preset["post"]["reconstructarcs"],
    refillAllZones=preset["post"]["refillzones"],
)
