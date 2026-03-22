
1. Identify the chain using OpenSSL
````
   openssl s_client -connect api.weatherapi.com:443 -servername api.weatherapi.com -showcerts
````
````
   openssl s_client -connect api.openweathermap.org:443 -servername api.openweathermap.org -showcerts
````
2. Test certificate if can connect
````
    openssl s_client \
    -connect api.weatherapi.com:443 \
    -servername api.weatherapi.com \
    -CAfile /mnt/c/project/wolfssl_20-3-2026/certs/openweather_root/ISRG_Root_X1.pem
````
````
    openssl s_client \
    -connect api.openweathermap.org:443 \
    -servername api.openweathermap.org \
    -CAfile /mnt/c/project/wolfssl_20-3-2026/certs/openweather_root/openweather_usertrust__root.crt
````
3. Examine certificate  
````
    openssl x509 -in openweather_root.crt -noout -subject -issuer
````