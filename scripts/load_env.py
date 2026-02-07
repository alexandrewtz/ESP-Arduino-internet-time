Import("env")

from pathlib import Path


def read_dotenv(path: Path) -> dict:
    data = {}
    try:
        content = path.read_text(encoding="utf-8")
    except OSError:
        return data

    for line in content.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key:
            data[key] = value
    return data


project_dir = Path(env["PROJECT_DIR"])
config = read_dotenv(project_dir / ".env")

ssid = config.get("WIFI_SSID") or env["ENV"].get("WIFI_SSID") or "YOUR_WIFI_SSID"
password = config.get("WIFI_PASSWORD") or env["ENV"].get("WIFI_PASSWORD") or "YOUR_WIFI_PASSWORD"

env.Append(
    BUILD_FLAGS=[
        f'-DWIFI_SSID=\\"{ssid}\\"',
        f'-DWIFI_PASSWORD=\\"{password}\\"',
    ]
)
