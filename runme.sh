#!/usr/bin/env sh

make lockushka
rm -f report.text

identifiers=""
amount_tasks=10

for step in $(seq 1 "${amount_tasks}"); do
  ./lockushka &
  printf "Launched %i\n" "${!}"
  identifiers="$identifiers ${!}"
done

identifiers="${identifiers# }"
sleep "$((60 * 1))"

set -- ${identifiers}

for identifier in "${@}"; do
  printf "Stop %i\n" "${identifier}"
  kill -2 "${identifier}"
done

sleep 30

amount_lines="$(wc --lines report.text | awk '{print $1}')"
amount_tries=0

while [ "${amount_lines}" -ne "${amount_tasks}" ]; do
  sleep 1
  amount_lines="$(wc --lines report.text | awk '{print $1}')"
  amount_tries="$((amount_tries + 1))"

  if [ "${amount_tries}" -ge 10 ]; then
    echo "amount_tasks != amount_lines"
    break
  fi
done

cat report.text
