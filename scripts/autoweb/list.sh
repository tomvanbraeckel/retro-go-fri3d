curl 'http://192.168.4.1/api' --data-raw '{"cmd":"list","arg1":"/sd","arg2":""}' \


#curl 'http://192.168.4.1/api' --data-raw '{"cmd":"list","arg1":"/sd/retro-go","arg2":""}' \

#curl 'http://192.168.4.1/api'  --data-raw '{"cmd":"list","arg1":"/sd/retro-go/saves/gbc/homebrew","arg2":""}' \

# huge:
#time curl -v 'http://192.168.4.1/api'  --data-raw '{"cmd":"list","arg1":"/sd//roms/gb/collection","arg2":""}'

time curl -v 'http://192.168.4.1/api'  --data-raw '{"cmd":"list","arg1":"/sd//roms/gbc/collection","arg2":""}'
