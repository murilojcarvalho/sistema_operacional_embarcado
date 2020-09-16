# -*- coding: cp1252 -*-
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import requests
import sys
import mysql.connector
import datetime 
import time

# Mosquitto MQTT
username    = "admin"
password    = "54321"
hostname    = "localhost"
port        = 1883


# Banco de dados MySql
user_mysql  = "root"
pwd_mysql   = "curso123"
db_mysql    = "registros" #tabela do banco de dados
led_name = '0'
led_value = '0'
Idx = 0

def on_connect(client, userdata, flags, rc):
    print("rc: " + str(rc))

def on_message(client, obj, msg): #alguem realizou uma publicação
    # Imprime o conteudo da messagem.
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload) )
    
    cnx = mysql.connector.connect(user=user_mysql, password=pwd_mysql, database=db_mysql)
    cursor = cnx.cursor()
    
    query = ("DELETE FROM sensores WHERE sensor_name = %s")
    dados_recebidos = (msg.topic, )
    print("Deleting data base ...")
    cursor.execute(query, dados_recebidos)
    
    query = ("INSERT INTO sensores " #prepara as informações(formata) e passa para variável query. 
            "(sensor_name, sensor_value, sensor_datetime)"
            "VALUES (%s, %s, %s)")
   
    dados_recebidos = (msg.topic, msg.payload, str(datetime.datetime.now()))#aqui é passado a publicação para uma variável 
    

    cursor.execute(query, dados_recebidos)# salva as informações
    cnx.commit() #salva o conteudo no banco
      
    # Encerra a conexÃ£o com o banco de dados.
    cursor.close()
    cnx.close()
       
def on_publish(client, obj, mid):
    print("mid: " + str(mid))

def on_subscribe(client, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

def on_log(client, obj, level, string):
    print(string)

def open_bd():  
    
    cnx = mysql.connector.connect(user=user_mysql, password=pwd_mysql, database=db_mysql)
    cursor = cnx.cursor()
    query_select = "SELECT * FROM atuadores" #abre a tabela leds

    cursor = cnx.cursor()
    cursor.execute(query_select) #executa a tabela
    records = cursor.fetchall()
    print("Total number of rows in atuadores is: ", cursor.rowcount) # conta as linhas
    global Idx
    for column in records:
        Id = column[0]
        led_name = column[1]
        led_value = column[2]
        led_datetime = column[3]   
    print("Idx =", Idx)
    print("Id = ", column[0])
    print("led_name = ", column[1])
    print("led_value = ", column[2])
    print("led_datetime  = ", column[3], "\n")
    # Encerra a conexÃ£o com o banco de dados.
    cursor.close()
    cnx.close()
    
    if Id > Idx: #se o indice atual for maior que o indice anterior publica
        mqttc.publish(led_name, led_value)
        Idx = Id
    

#------------------------------------------------
mqttc = mqtt.Client()

# Assina as funÃ§Ãµes de callback
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe
 
mqttc.username_pw_set(username, password)

mqttc.connect(hostname, port)

mqttc.subscribe("#", 0)

led1= "led1"


# Permanece em loop mesmo se houver erros. 
while True:
    rc = 0
    while rc == 0:
        rc = mqttc.loop()
        time.sleep(3)
        open_bd()
    break
