#!/bin/python3

import subprocess
import time
import socket
import signal
import glob
import os, shutil
import sys

def session(name, host, port, first_answer, session):
    errors = []
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((host, port))
    s.listen(10)
    c, addr = s.accept()
    c.sendall(first_answer)
    for step in session:
        try:
            if (len(step) == 2):
                answer = c.recv(1024)
                c.sendall(step[1])
                assert answer == step[0], "Пришёл неожиданный запрос: %s ( != %s)" % (answer, step[0])
            else:
                answer = c.recv(1024)
                assert answer == step, "Пришло неожиданное содержание письма: %s ( != %s)" % (answer, step)
        except AssertionError as err:
            print("Тест провален:\n\t", err)
            errors.append(err)
    c.close()
    s.close()
    return errors;

def copy_newest_file(file1, file2):
    res = subprocess.run(["cp", file1, file2])
    return not res.returncode

def clean_dir(path):
    files = glob.glob(path)
    for f in files:
        if os.path.isdir(f):
            shutil.rmtree(f)
        elif os.path.isfile or os.path.islink:
            os.unlink(f)

def main(path):
    HOST = '127.0.0.1'
    PORT = 25

    clean_dir("./tmp/*")
    subprocess.run(["mkdir", "./tmp/server"])

    first_answer = b'220 Service ready\r\n\x00'
    success_session =  [(b'EHLO mysmtp\r\n', b'250 OK\r\n\x00'),
                (b'MAIL FROM:<zhar97@yandex.ru>\r\n', b'250 OK\r\n\x00'),
                (b'RCPT TO:<aa@[127.0.0.1]>\r\n', b'250 OK\r\n\x00'),
                (b'RCPT TO:<bb@[127.0.0.1]>\r\n', b'250 OK\r\n\x00'),
                (b'DATA\r\n', b'354 Start mail input\r\n\x00'),
                (b'hello server\r\n'),
                (b'.\r\n', b'250 OK\r\n\x00'),
                (b'QUIT\r\n', b'221 Bye\r\n\x00')]
    
    client = subprocess.Popen(path, stderr=subprocess.DEVNULL, shell=False)
    errors = []
    try:
        assert client.poll() is  None, "Клиент неожиданно завершился"
        copy_newest_file("./expected_mail.txt", "./tmp/server/127.0.0.1.0.id")
        errors = session("success_session", HOST, PORT, first_answer, success_session)
    except AssertionError as err:
        print("Тест провален:\n\t", err)
        errors.append(err)
    finally:
        client.send_signal(signal.SIGINT)
        client.wait()

    if len(errors) > 0:
        print("Тест провален:\n\t%s завершилась с %d ошибками." % (path, len(errors)))
        exit(1)
    else:
        print("Teсты пройдены успешно.")

if __name__ == "__main__":
    path = "./run_client.sh"
    if len(sys.argv) == 2:
        path = sys.argv[1]
    main(path)
