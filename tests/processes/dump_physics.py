from __future__ import annotations

from remage import remage_run


def run_sim(run_name: str, phys_commands: str):
    output_file = f"processes.{run_name}.output.txt"
    macro = [
        *phys_commands,
        "/run/initialize",
        f"/RMG/Processes/DumpProcessesForParticles {output_file}",
    ]

    remage_run(
        macro,
        gdml_files="gdml/geometry.gdml",
    )


# Define the different detector registration commands to test
runs = [
    ("default", []),
    ("optical", ["/RMG/Processes/OpticalPhysics"]),
    ("hadr-shielding", ["/RMG/Processes/HadronicPhysics Shielding"]),
    ("innerbrem", ["/RMG/Processes/EnableInnerBremsstrahlung"]),
    (
        "grabmayr",
        [
            "/RMG/Processes/HadronicPhysics Shielding",
            "/RMG/Processes/UseGrabmayrsGammaCascades",
        ],
    ),
]

# Run each simulation
for run_name, commands in runs:
    run_sim(run_name, commands)
