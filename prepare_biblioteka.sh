#!/bin/bash
input="mock_data.txt"
while IFS= read -r var
do
  if [ ! -f ./biblioteka/$var.txt ]; then
      echo "$var"
      echo "https://pl.wikipedia.org/wiki/$var"
      curl "https://pl.wikipedia.org/wiki/$var" | html2text -utf8 -o ./biblioteka/$var.txt
  fi
done < "$input"