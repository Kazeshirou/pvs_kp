#!/bin/python3

import subprocess
import time
import socket
import signal
import glob
import os, shutil
import sys

def session(name, host, port, first_answer, session):
    errors = None
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((host, port))
        answer = s.recv(1024)
        assert answer == first_answer, "Пришло неожиданное приветствие сервера: %s ( != %s)" % (answer, first_answer)
        for step in session:
            if (len(step) == 2):
                s.sendall(step[0])
                answer = s.recv(1024)
                assert answer == step[1], "Пришёл неожиданный ответ на %s: %s ( != %s)" % (step[0], answer, step[1])
            elif (len(step) == 3):
                sleep(step[2])
                s.sendall(step[0])
                answer = s.recv(1024)
                assert answer == step[1], "Пришёл неожиданный ответ на %s: %s ( != %s)" % (step[0], answer, step[1])
            else:
                s.sendall(step)
    except AssertionError as err:
        errors = err
    finally:
        s.close()

    if errors is not None:
        assert False, errors

def files_is_equal(file1, file2):
    res = subprocess.run(["cmp", file1, file2], capture_output=True)
    return not res.returncode

def get_newest_file(path):
    files = glob.glob(path)
    if not len(files):
        return ""

    return max(files, key=os.path.getctime)

def clean_dir(path):
    files = glob.glob(path)
    for f in files:
        if os.path.isdir(f):
            shutil.rmtree(f)
        elif os.path.isfile or os.path.islink:
            os.unlink(f)

def main(path):
    HOST = '127.0.0.1'
    PORT = 64999

    clean_dir("tests/tmp/*") 

    first_answer = b'220 Service ready\r\n'
    success_session =  [(b'EHLO mysmtp\r\n', b'250 OK\r\n'),
                (b'mail from:<zhar97@yandex.ru>\r\n', b'250 OK\r\n'),
                (b'rcpt to:<aa@mysmtp.ru>\r\n', b'250 OK\r\n'),
                (b'rcpt to:<aa@ya.ru>\r\n', b'250 OK\r\n'),
                (b'data\r\n', b'354 Start mail input\r\n'),
                (b'helo server\r\n'),
                (b'.\r\n', b'250 OK\r\n'),
                (b'quit\r\n', b'221 Bye\r\n')]
    
    server = subprocess.Popen(path, stderr=subprocess.DEVNULL, shell=False)
    time.sleep(1)
    errors = False
    try:
        assert server.poll() is  None, "Сервер неожиданно завершился"

        create_client = subprocess.run(["./create_client.sh", "tests/tmp/server", "aa"], capture_output=True)
        assert not create_client.returncode, "Не удалось создать папку пользователя %s" % create_client.stderr.decode("utf-8") 

        session("success_session", HOST,PORT, first_answer, success_session)

        latest_file = get_newest_file("tests/tmp/client/ya.ru.2.*")
        assert len(latest_file), "Письмо в папке клиента не создано"
        assert files_is_equal(latest_file, "tests/expected_mail.txt"), "Письмо в папке клиента не совпадает с ожидаемым"
    except AssertionError as err:
        print("Тест провален:\n\t", err)
        errors = True
    finally:
        server.send_signal(signal.SIGINT)
        server.wait()

    if server.returncode:
        print("Тест провален:\n\t%s завершилась с ошибками." % path)
        errors = True

    if errors:
        exit(1)
    else:
        print("Teсты пройдены успешно.")

if __name__ == "__main__":
    path = "./tests/run_server.sh"
    if len(sys.argv) == 2:
        path = sys.argv[1]
    main(path)