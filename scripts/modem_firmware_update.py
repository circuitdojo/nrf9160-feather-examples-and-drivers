from pyocd.target.family.target_nRF91 import ModemUpdater
from pyocd.core.helpers import ConnectHelper
from pyocd.flash.file_programmer import FileProgrammer
import sys, os

# Get the first argument
file_path = sys.argv[1]

# Check if the argument is a file
if not os.path.isfile(file_path):
    print("File not found")
    sys.exit()

with ConnectHelper.session_with_chosen_probe(options = {"frequency": 12000000, "target_override": "nrf91"}) as session:
    # Use file path here
    ModemUpdater(session).program_and_verify(file_path)