#!/bin/bash

if [ $# != 1 ]
then
echo "Please provide the number of tests to perform."
exit 1
fi

NB_TEST=$1								# number of tests to perform for each configuration
DIRECTORY=/tmp						# working directory where files will be saved
EXEC="./resol"						# executable
NVAR=30
OUTPUT=clauses.nclause.dat

rm -f $OUTPUT
rm -f $DIRECTORY/output.txt
echo "Clause_number DUMB RAND MOMS DLIS" >> $OUTPUT

for nb in `seq 1 40`; do
	TIME_DUMB=0
	TIME_RAND=0
	TIME_MOMS=0
	TIME_DLIS=0
	A=`expr $nb \* 5`
  echo "Computing test for " $A " clauses."

	# On fait plusieurs tests par taille

	for test in `seq 1 $NB_TEST` ; do
		echo -e "\t\tTest $test"
		# Génération de la formule dans le fichier $DIRECTORY/formula.cnf
		./generator -nvar $NVAR -nclause $A -sclause 3 -o $DIRECTORY/formula.cnf -pathologic

		# Résolution de la formule (en vérifiant la correction de la solution)
	
		# Heuristique DUMB
		/usr/bin/time --quiet -f'%U' -o $DIRECTORY/result.txt $EXEC $DIRECTORY/formula.cnf | ./check_result.py $DIRECTORY/formula.cnf >> $DIRECTORY/output.txt
		TMP=`cat $DIRECTORY/result.txt`
		TIME_DUMB=$(echo "scale=3; $TIME_DUMB + $TMP" | bc)
		# Heuristique RAND
		/usr/bin/time --quiet -f'%U' -o $DIRECTORY/result.txt $EXEC -rand $DIRECTORY/formula.cnf | ./check_result.py $DIRECTORY/formula.cnf >> $DIRECTORY/output.txt
		TMP=`cat $DIRECTORY/result.txt`
		TIME_RAND=$(echo "scale=3; $TIME_RAND + $TMP" | bc)
		# Heuristique MOMS
		/usr/bin/time --quiet -f'%U' -o $DIRECTORY/result.txt $EXEC -moms $DIRECTORY/formula.cnf | ./check_result.py $DIRECTORY/formula.cnf >> $DIRECTORY/output.txt
		TMP=`cat $DIRECTORY/result.txt`
		TIME_MOMS=$(echo "scale=3; $TIME_MOMS + $TMP" | bc)
		# Heuristique DLIS
		/usr/bin/time --quiet -f'%U' -o $DIRECTORY/result.txt $EXEC -dlis $DIRECTORY/formula.cnf | ./check_result.py $DIRECTORY/formula.cnf >> $DIRECTORY/output.txt
		TMP=`cat $DIRECTORY/result.txt`
		TIME_DLIS=$(echo "scale=3; $TIME_DLIS + $TMP" | bc)
		
	done
	TIME_DUMB=$(echo "scale=3; $TIME_DUMB / $NB_TEST" | bc)
	TIME_RAND=$(echo "scale=3; $TIME_RAND / $NB_TEST" | bc)	
	TIME_MOMS=$(echo "scale=3; $TIME_MOMS / $NB_TEST" | bc)	
	TIME_DLIS=$(echo "scale=3; $TIME_DLIS / $NB_TEST" | bc)	
	echo $A $TIME_DUMB $TIME_RAND $TIME_MOMS $TIME_DLIS >> $OUTPUT
# fin de la boucle
done

# gnuplot script-plot.p

# evince courbe1.pdf &

# Si une formule n'était pas satisfaite par l'affectation, on aura des "assert" dans le fichier
#grep "assert" $DIRECTORY/output.txt
