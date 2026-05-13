import xml.etree.ElementTree as ET
for fname in ["ui/firmwareupdatedialog.ui", "ui/mainwindow.ui"]:
    try:
        ET.parse(fname)
        print(f"{fname}: OK")
    except ET.ParseError as e:
        print(f"{fname}: {e}")
