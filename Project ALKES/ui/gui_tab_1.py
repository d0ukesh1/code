from PyQt6.QtWidgets import QWidget, QHBoxLayout, QPushButton, QLabel, QFileDialog, QMessageBox, QVBoxLayout, QFrame, QComboBox
from data_processing import *
import os

class Tab1Content(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.check_filename = None
        self.computers = {} 
        self.setup_ui()
        self.editor = None
        self.pc_name_combo.currentIndexChanged.connect(self.update_fields)

    def setup_ui(self):
        """ Настроим содержимое для первой вкладки """
        self.verticalLayout = QVBoxLayout(self)

        # Метка и кнопка для выбора файла Проверка
        self.label = QLabel("Выберите файл Проверка", self)
        self.label.setWordWrap(True)
        self.button = QPushButton("Выбрать", self)
        self.button.clicked.connect(self.check_file_button)

        # Размещение label и button на вертикальном лейауте
        self.horizontalLayout1 = QHBoxLayout()
        self.horizontalLayout1.addWidget(self.label)
        self.horizontalLayout1.addWidget(self.button)
        self.verticalLayout.addLayout(self.horizontalLayout1)

        # Дополнительная метка и кнопка для другого файла
        self.label1 = QLabel("Выберите Excel-файл, который нужно обработать", self)
        self.label1.setWordWrap(True)
        self.button1 = QPushButton("Выбрать")
        self.button1.clicked.connect(self.file_button_1)

        # Размещение второй метки и кнопки на вертикальном лейауте
        self.horizontalLayout2 = QHBoxLayout()
        self.horizontalLayout2.addWidget(self.label1)
        self.horizontalLayout2.addWidget(self.button1)
        self.verticalLayout.addLayout(self.horizontalLayout2)

        # Кнопка для выбора файла с компьютерами
        self.label2 = QLabel("Выберите файл для изменения данных", self)
        self.label2.setWordWrap(True)
        self.button2 = QPushButton("Выбрать")
        self.button2.clicked.connect(self.file_button_2)

        self.horizontalLayout3 = QHBoxLayout()
        self.horizontalLayout3.addWidget(self.label2)
        self.horizontalLayout3.addWidget(self.button2)
        self.verticalLayout.addLayout(self.horizontalLayout3)

        # Новый фрейм для отображения полей
        self.fields_frame = QFrame(self)
        self.fields_layout = QVBoxLayout(self.fields_frame)  # Вертикальный лейаут для всех полей
        self.fields_frame.setLayout(self.fields_layout)

        # 1. Выпадающий список для выбора ПК
        self.pc_name_label = QLabel("Имя ПК", self)
        self.pc_name_combo = QComboBox(self)
        self.pc_name_combo.addItem("Выберите ПК")
        self.pc_name_combo.setEnabled(False)  # Пока ПК не выбран, поле отключено
        self.fields_layout.addWidget(self.pc_name_label)
        self.fields_layout.addWidget(self.pc_name_combo)

        # 2. Выпадающий список для выбора статуса
        self.status_label = QLabel("Выбрать статус", self)
        self.status_combo = QComboBox(self)
        self.status_combo.addItems(Computer.STATUS_OPTIONS)
        self.status_combo.setEnabled(False)  # Пока ПК не выбран, поле отключено
        self.fields_layout.addWidget(self.status_label)
        self.fields_layout.addWidget(self.status_combo)

        # 3. Выпадающий список для выбора браузера
        self.browser_label = QLabel("Обнаруженный браузер", self)
        self.browser_combo = QComboBox(self)
        self.browser_combo.addItems(Computer.BROWSER_OPTIONS)
        self.browser_combo.setEnabled(False)  # Пока ПК не выбран, поле отключено
        self.fields_layout.addWidget(self.browser_label)
        self.fields_layout.addWidget(self.browser_combo)

        # Кнопка для изменения данных
        self.change_button = QPushButton("Изменить данные", self)
        self.change_button.setEnabled(False)  # Кнопка недоступна до выбора ПК и заполнения полей
        self.change_button.clicked.connect(self.change_data)
        self.fields_layout.addWidget(self.change_button)

        # Скрываем фрейм с полями до выбора файла
        self.fields_frame.setVisible(False)

        # Добавляем фрейм в основной лейаут
        self.verticalLayout.addWidget(self.fields_frame)
        self.verticalLayout.addStretch()

    def check_file_button(self):
        """ Обрабатывает выбор файла Проверка """
        try:
            self.check_filename, _ = QFileDialog.getOpenFileName(self, "Выбрать Excel файл", "", "Excel Files (*.xlsx; *.xls);;All Files (*)")
            
            if self.check_filename:
                Logger.log_info(f"Файл {self.check_filename} выбран.")
                self.label.setText(f"Файл выбран: {os.path.basename(self.check_filename)}")
            else:
                Logger.log_error("Файл Проверка не выбран.")
                self.label.setText("Файл не выбран.")
            
        except Exception as e:
            Logger.log_error(f"Ошибка чтения файла Проверка: {str(e)}")
            self.label.setText(f"Ошибка: {str(e)}")

    def file_button_1(self):
        """ Открывает диалог для выбора другого файла и обрабатывает его. """
        try:
            filename, _ = QFileDialog.getOpenFileName(self, "Выбрать Excel файл", "", "Excel Files (*.xlsx; *.xls);;All Files (*)")
            
            if not filename:
                self.label1.setText("Файл не выбран.")
                Logger.log_error("Файл на обработку не выбран.")
                return
            # Извлечение даты из имени файла
            match = re.search(r"\d{4}[-_]\d{2}[-_]\d{2}", filename)
            extract_date_value = match.group(0) if match else None

            if not extract_date_value:
                Logger.log_error(f"Дата не найдена в названии файла: {filename}. Файл Проверка не будет создан.")
                self.label1.setText("Дата не найдена в названии файла.")
                return
            
            # Инициализируем TableEditor для выбранного файла
            editor1 = TableEditor(filename)
            editor1.default_edit()  # Применяем нужное форматирование

            # Добавляем столбцы, если их нет
            ADD_COLUMN_1 = "АВЗ (Целевая версия установлена, базы обновлены, политики применены.) СТАТУС НА МОМЕНТ ПРОВЕРКИ"
            ADD_COLUMN_2 = "Некорпоративные браузеры"
            editor1.add_column_if_missing(ADD_COLUMN_1)
            editor1.add_column_if_missing(ADD_COLUMN_2)
            editor1.format_table()

            ch_filename = f"{extract_date_value}_Проверка.txt"

            # Проверяем, что check_filename был выбран
            if self.check_filename:
                self.editor = TableEditor(self.check_filename)  # Используем файл проверки
                pc_names = list(Computer.load_computers(filename,create_files = True).keys())  # Извлекаем имена ПК
                check_pc_name = self.editor.find_elements_in_checkfile(pc_names)

                with open(ch_filename, "w", encoding="utf-8") as ch_file:
                    for computer in check_pc_name:
                        ch_file.write(computer + "\n")

                self.label1.setText(f"Файл с датой {extract_date_value} обработан, приступайте к следующему шагу.")
            else:
                self.label1.setText("Файл Проверка не инициализирован. Пожалуйста, выберите файл.")
        except Exception as e:
            self.label1.setText(f"Ошибка: {str(e)}")
            Logger.log_error(f"Ошибка в обработке файла: {str(e)}")

    def file_button_2(self):
        """ Открывает диалог для выбора файла с компьютерами или сбрасывает данные, если файл уже выбран """
        # Если файл уже выбран, сбрасываем данные и меняем текст кнопки
        if self.computers:
            self.reset_file_selection()  # Сбрасываем файл и интерфейсные данные

        else:
            try:
                filename, _ = QFileDialog.getOpenFileName(self, "Выбрать Excel файл", "", "Excel Files (*.xlsx; *.xls);;All Files (*)")

                if not filename:
                    Logger.log_error("Файл не выбран.")
                    self.label2.setText("Файл не выбран.")
                    return

                Logger.log_info(f"Загружаем данные из файла: {filename}")
            
                self.current_filename = filename # Сохраняем имя текущего файла
                self.computers = Computer.load_computers(filename)  # Загружаем данные о компьютерах
                
                if not self.computers:
                    Logger.log_error(f"Не удалось загрузить данные из файла: {filename}")
                    self.label2.setText("Не удалось загрузить данные.")
                    return
                
                # Сохраняем TableEditor для использования в change_data
                self.editor = TableEditor(filename)
                # Настраиваем интерфейс для работы с выбранными данными
                self.setup_computer_selection()
                self.fields_frame.setVisible(True)
                self.button2.setText("Закрыть файл")  # Меняем текст кнопки
                self.label2.setText(f"Файл выбран: {os.path.basename(filename)}")
                Logger.log_info("Данные успешно загружены и интерфейс обновлен.")

            except Exception as e:
                Logger.log_error(f"Ошибка при обработке файла: {str(e)}")
                self.label2.setText(f"Ошибка: {str(e)}")
                QMessageBox.critical(self, "Ошибка", f"Произошла ошибка: {str(e)}")

    def reset_file_selection(self):
        """ Полностью сбрасывает выбор файла и очищает все данные о компьютерах и интерфейсные элементы """
        
        self.editor = None
        # Очищаем объект self.computers полностью
        self.computers.clear()
        self.computers = {}

        # Скрываем фрейм с полями
        self.fields_frame.setVisible(False)

        # Сброс выпадающего списка для ПК
        self.pc_name_combo.blockSignals(True)
        self.pc_name_combo.clear()
        self.pc_name_combo.addItem("Выберите ПК")
        self.pc_name_combo.setEnabled(False)
        self.pc_name_combo.blockSignals(False)

        # Сброс выпадающего списка для статуса
        self.status_combo.blockSignals(True)
        self.status_combo.clear()
        self.status_combo.addItems(Computer.STATUS_OPTIONS)
        self.status_combo.setEnabled(False)
        self.status_combo.blockSignals(False)

        # Сброс выпадающего списка для браузера
        self.browser_combo.blockSignals(True)
        self.browser_combo.clear()
        self.browser_combo.addItems(Computer.BROWSER_OPTIONS)
        self.browser_combo.setEnabled(False)
        self.browser_combo.blockSignals(False)

        # Отключаем кнопку изменения данных
        self.change_button.setEnabled(False)

        # Сброс метки файла
        self.label2.setText("Выберите файл для изменения данных")
        self.button2.setText("Выбрать файл")  # Возвращаем текст кнопки
        Logger.log_info("Файл закрыт, данные о компьютерах и интерфейс сброшены.")

        # Принудительное обновление интерфейса
        self.repaint()

    def setup_computer_selection(self):
        """ Настроить UI для выбора ПК, статуса и браузера """
        # Очистка и настройка выпадающего списка
        self.pc_name_combo.blockSignals(True)  # Блокируем сигналы во время обновления
        self.pc_name_combo.clear()
        self.pc_name_combo.addItem("Выберите ПК")
        
        # Получаем имена ПК из текущих данных и добавляем их в список
        pc_names = list(self.computers.keys())
        self.pc_name_combo.addItems(pc_names)
        self.pc_name_combo.blockSignals(False)  # Разблокируем сигналы после обновления

        # Активируем поля
        self.pc_name_combo.setEnabled(True)
        self.status_combo.setEnabled(True)
        self.browser_combo.setEnabled(True)

        # Изначально статус и браузер будут не активны
        self.status_combo.setEnabled(False)
        self.browser_combo.setEnabled(False)

    def update_fields(self):
        """ Обновляем доступность полей на основе выбора ПК """
        selected_pc_name = self.pc_name_combo.currentText()

        if selected_pc_name != "Выберите ПК":
            # Получаем объект компьютера из словаря
            selected_computer = self.computers[selected_pc_name]

            # Устанавливаем текущие значения для статуса
            self.status_combo.setEnabled(True)
            self.browser_combo.setCurrentText(selected_computer.browser)
            self.browser_combo.setEnabled(True)  # Включаем поле для браузера
            self.browser_combo.setVisible(True)  # Показываем выпадающий список браузеров
            self.status_combo.setCurrentText(selected_computer.status)
            self.change_button.setEnabled(True)
        else:
            # Если не выбран ПК, то блокируем остальные поля
            self.status_combo.setEnabled(False)
            self.browser_combo.setEnabled(False)
            self.change_button.setEnabled(False)

    def change_data(self):
        """Изменяет статус и браузер для выбранного ПК и записывает в файл."""
        selected_pc_name = self.pc_name_combo.currentText()
        
        # Если выбран ПК
        if selected_pc_name != "Выберите ПК":
            filename = self.current_filename
            Logger.log_info(f"Пытаемся найти ПК {selected_pc_name} в файле {os.path.basename(filename)}")

            status = self.status_combo.currentText()
            browser = self.browser_combo.currentText()

            # Инициализация индексов для столбцов
            pc_name_column = None
            status_column = None
            browser_column = None

            # Определяем индексы столбцов по их заголовкам
            for col in range(1, self.editor.sheet.max_column + 1):
                cell_value = self.editor.sheet.cell(row=1, column=col).value
                if cell_value == "ИМЯ ПК":
                    pc_name_column = col
                elif cell_value == "АВЗ (Целевая версия установлена, базы обновлены, политики применены.) СТАТУС НА МОМЕНТ ПРОВЕРКИ":
                    status_column = col
                elif cell_value == "Некорпоративные браузеры":
                    browser_column = col

            # Проверяем, что нужные столбцы найдены
            if pc_name_column is None or status_column is None or browser_column is None:
                QMessageBox.warning(self, "Ошибка", "Не удалось найти необходимые столбцы в файле.")
                return

            # Проходим по всем строкам, заполняем пустые ячейки значениями по умолчанию
            for i in range(2, self.editor.sheet.max_row + 1):
                pc_name = self.editor.sheet.cell(row=i, column=pc_name_column).value
                if pc_name:  # Если имя ПК не пустое
                    # Проверяем статус и браузер, если они пустые, заполняем их значениями по умолчанию
                    if not self.editor.sheet.cell(row=i, column=status_column).value:
                        self.editor.fill_cell(i, status_column, "Ок")
                    if not self.editor.sheet.cell(row=i, column=browser_column).value:
                        self.editor.fill_cell(i, browser_column, "Отсутствуют")

            # Ищем строку для выбранного ПК
            row = None
            for i in range(2, self.editor.sheet.max_row + 1):
                if self.editor.sheet.cell(row=i, column=pc_name_column).value == selected_pc_name:
                    row = i
                    break

            if row is None:
                QMessageBox.warning(self, "Ошибка", "Компьютер не найден в файле.")
                return

            # Записываем выбранные значения в ячейки
            self.editor.fill_cell(row, status_column, status)
            self.editor.fill_cell(row, browser_column, browser)

            # Если выбранный статус не "Ок", окрашиваем текст в красный
            if status != "Ок":
                self.editor.highlight_text_in_red(row)

            Logger.log_info(f"Данные для ПК {selected_pc_name} изменены: статус = {status},{f"браузер = {browser}" if status == "Не согласовано/Обнаружен браузер" else " "}")
            QMessageBox.information(self, "Данные изменены", f"Данные для ПК {selected_pc_name} успешно изменены.")
        else:
            QMessageBox.warning(self, "Ошибка", "Пожалуйста, выберите ПК для изменения.")

