# pyinstaller --onefile --noconsole --hidden-import openpyxl --add-data "ui/alrosa.ico;ui" main.py
# python build.py

import subprocess
import shutil, os

# Укажите путь к вашему скрипту
script_name = 'main.py'
icon_path = 'ui/alrosa.ico'
anim_png = 'ui/alrosa_start.png'

# Команда для PyInstaller
command = [
    'pyinstaller',
    '--onefile',
    '--noconsole',
    '--hidden-import=openpyxl',
    f'--add-data={icon_path};ui',
    f'--add-data={anim_png};ui',
    script_name
]

# Запускаем PyInstaller
subprocess.run(command)

# Опционально: удалите временные файлы, созданные PyInstaller
if os.path.exists('build'):
    shutil.rmtree('build')  # Удаляет директорию и все её содержимое
spec_file = script_name.replace('.py', '.spec')
if os.path.exists(spec_file):
    os.remove(spec_file)