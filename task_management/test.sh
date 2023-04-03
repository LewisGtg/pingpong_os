#/bin/bash

make

tests="teste1 teste2 teste3"
counter=1

for test in $tests
do
    ./$test > temp
    out=$(diff temp expected$counter.txt)

    if [$out -eq ""]
    then
        echo "Teste $counter sem erros!"
    else
        diff temp expected$counter.txt
    fi

    counter=$((counter+1))
done

rm temp