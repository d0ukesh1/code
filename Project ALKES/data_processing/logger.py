# data_processing/logger.py

import datetime

class Logger:
    log_file_path = "service.log"

    @staticmethod
    def log(message, level="INFO"):
        """Записывает сообщение в лог-файл с заданным уровнем важности."""
        log_message = f"{datetime.datetime.now():%Y-%m-%d %H:%M:%S} [{level}] {message}"
        
        try:
            # Запись сообщения в файл (append - добавление в конец файла)
            with open(Logger.log_file_path, "a", encoding="utf-8") as writer:
                writer.write(log_message + "\n")
        except Exception as ex:
            print(f"Не удалось записать лог: {ex}")

    @staticmethod
    def log_info(message):
        """Записывает информационное сообщение в лог."""
        Logger.log(message, "INFO")

    @staticmethod
    def log_warning(message):
        """Записывает предупреждающее сообщение в лог."""
        Logger.log(message, "WARNING")

    @staticmethod
    def log_error(message):
        """Записывает сообщение об ошибке в лог."""
        Logger.log(message, "ERROR")