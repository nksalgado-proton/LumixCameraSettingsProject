"""Register Lightning Alert as a Windows Scheduled Task (runs on login)."""

import subprocess
import sys

TASK_NAME = "LightningAlert"
BAT_PATH = r"D:\Projetos_Nelson\LumixCameraSettingsProject\tools\lightning_alert\start_monitor.bat"

def install():
    cmd = [
        "schtasks", "/create",
        "/tn", TASK_NAME,
        "/tr", BAT_PATH,
        "/sc", "onlogon",
        "/rl", "highest",
        "/f",
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, shell=True)
    if result.returncode == 0:
        print(f"Tarefa '{TASK_NAME}' criada com sucesso!")
        print("O monitor vai iniciar automaticamente no próximo login.")
    else:
        print(f"Erro: {result.stderr}")
        print("Tente executar como Administrador.")

def uninstall():
    cmd = ["schtasks", "/delete", "/tn", TASK_NAME, "/f"]
    result = subprocess.run(cmd, capture_output=True, text=True, shell=True)
    if result.returncode == 0:
        print(f"Tarefa '{TASK_NAME}' removida.")
    else:
        print(f"Erro: {result.stderr}")

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "remove":
        uninstall()
    else:
        install()
