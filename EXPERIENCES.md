Comparaison des performances RENDU 2
====================================

Nous avons implémenté un programme en OCaml, permettant de générer aléatoirement 
un ensemble de clauses, une formule de logique propositionnelle, ou un graphe.
  * Compilation :
    make generator (ou simplement « make », qui compile tout le projet)
  * Génération d'un ensemble de 10 clauses de taille 3, pour 5 variables :   
    ./generator -nvar 5 -nclause 10 -sclause 3
  * Génération d'une formule de profondeur 3, pour 5 variables :             
    ./generator -nvar 5 -depth 3
  * Génération d'un graphe de 5 noeuds, et 10 arêtes :                       
    ./generator -nvar 5 -nedge 10
Ces commandes écriront l'objet voulu sur la sortie standard. Pour l'écrire dans 
un fichier "abc.txt", on peut rajouter l'option "-o abc.txt".
Nous avons remarqué, un peu tard, que notre générateur est trop lent pour 
générer de grands ensembles de clauses.

Nous avons aussi écrit quatre scripts bash permettant de lancer automatiquement 
des séries de résolutions.
./clause.nvar_3SAT.sh <int>
    Ensembles de clauses 3SAT, le nombre de variables varie.
    Nombre de clauses = nombre de variables * 4.26
./clause.nvar_5SAT.sh <int>
    Ensembles de clauses 3SAT, le nombre de variables varie.
    Nombre de clauses = nombre de variables * 21
./graph.nvar.sh <int>
    Graphe , le nombre de noeuds varie.
    Densité constante de 0.35.
./formulae.depth.sh <int>
    Formules propositionnelles, la profondeur varie.
L'entier passé en argument est le nombre de tests à exécuter pour chaque
configuration.
Ces différents benchmarks sont détaillés plus bas.

Après avoir compilé nos programmes comme il se doit (voir README), se placer
dans le répertoire build.
Les fichiers d'extensions .dat sont les résultats expérimentaux que nous avons
obtenus. Les courbes correspondantes sont dans le fichier courbes.pdf (situé 
dans le répertoire racine de ce projet).
Pour générer de nouveaux résultats, il suffit d'exécuter les scripts mentionnés
plus haut. Attention : cela écrasera les résultats expérimentaux présents dans
le répertoire build, il faut penser à les sauvegarder.
Pour générer un nouveau fichier courbes.pdf, lancer : "gnuplot script-plot.p".


Chaque résultat présenté a été calculé en faisant 100 tests, sur des objets 
générés aléatoirement avec les même paramètres, puis en prenant la moyenne des 
temps d'exécution.
Chaque courbe a comme axe des abscisses le paramètre que nous testons, et comme 
axe des ordonnées le temps d'exécution moyen (une échelle logarithmique est 
utilisée).
Nous avons testé à chaque fois les quatre heuristiques, avec et sans watched 
literals (donc 8 courbes par jeu de test).

Formules de logique propositionnelle
------------------------------------

Nous avons généré des formules de 5 variables, en faisant varier la profondeur.
La profondeur est défini de manière récursive, pour f1 et f2 des formules :
Profondeur(var) = 0
Profondeur(~f1) = 1+Profondeur(f1)
Profondeur(f1/\f2)   = Profondeur(f1\/f2) = Profondeur(f1=>f2) 
= 1 + max(Profondeur(f1),Profondeur(f2))
Comme nous pouvions nous y attendre, le temps d'exécution augmente de manière 
exponentielle par rapport à la profondeur.
De plus, en temps d'exécution moyen, on a :
DUMB_WL > DUMB > RAND > MOMS = DLIS > RAND_WL > MOMS_WL = DLIS_WL
Pour des formules de plus petite profondeur, DUMB > DUMB_WL
Ainsi, DLIS et MOMS semblent meilleures que les autres heuristiques. De plus, 
les watched literals apportent un gain significatif.

Ensembles de clauses 3SAT
-------------------------

Nous avons généré des ensembles de clauses de taille 3, en faisant varier le 
nombre N de variables, et en prenant ceil(4.26*N) clauses.
D'après ce que nous avons pu lire sur internet, c'est ainsi que l'on obtient les 
ensembles de clauses 3SAT les plus complexes à résoudre.
De plus, chaque clause contient le littéral N ou le littéral -N, ce qui nous 
semble être un cas pathologique pour l'heuristique DUMB.
On observe clairement ceci, en temps d'exécution :
DUMB > DUMB_WL > RAND > RAND_WL > MOMS >= DLIS > MOMS_WL = DLIS_WL
Ainsi, DLIS et MOMS sont plus performantes, et les watched literals apportent de 
nouveau un gain significatif.

Ensembles de clauses 5SAT
-------------------------

Nous avons généré des ensembles de clauses de taille 5, en faisant varier le 
nombre N de variables, et en prenant 21*N clauses.
Nous n'avons pas trouvé d'informations claires à ce sujet, donc nous avons 
obtenu le facteur 21 de manière expérimentale.
Pour ce faire, on fixe N, on fait varier le nombre M de clauses. Pour M donné, 
on fait 100 tests. Le nombre d'ensembles satisfiables diminue quand M augmente.
Il semble qu'environ 50% des ensembles soient satisfiables pour M=21*N.
De plus, chaque clause contient le littéral N ou le littéral -N, ce qui nous 
semble être un cas pathologique pour l'heuristique DUMB.
On observe clairement ceci, en temps d'exécution :
DUMB > DUMB_WL > RAND > RAND_WL > DLIS > MOMS > DLIS_WL > MOMS_WL
Ainsi, MOMS est l'heuristique la plus performante, suivie de DLIS, et les 
watched literals apportent de nouveau un gain significatif.
Le gain apporté par les watched literals semble plus important, ce qui parait 
logique en prenant des clauses de plus grandes tailles.

Graphes
-------

Nous avons généré des graphes de N noeuds, et ceil(0.35*N*(N-1)/2) arêtes, et 
nous essayons de les colorier avec 4 couleurs.
On obtient ainsi des graphes de densité environ 0.35 (la densité étant le nombre 
d'arêtes divisé par le nombre maximal d'arêtes, soit N*(N-1)/2).
La densité 0.35 a été obtenue expérimentalement, c'est ce qui semblait garantir 
environ 50% de graphes 4-coloriables.
On observe ceci en temps d'exécution :
DUMB > DUMB_WL >= RAND > RAND_WL > MOMS > MOMS_WL > DLIS > DLIS_WL
Ainsi, DLIS est l'heuristique la plus performante, suivie de MOMS. Les watched 
literals apportent de nouveau un grain significatif.

Conclusion
----------

Les watched literals semblent ne jamais entraîner de temps d'exécution plus 
important. Au contraire, le temps d'exécution est dans une majorité de cas 
strictement inférieur.
De plus, les heuristiques MOMS et DLIS sont largement plus performantes que les 
heuristiques DUMB et RAND.
L'heuristique RAND est meilleure que l'heuristique DUMB dans une majorité de cas.
Il est difficile de départager l'heuristique MOMS de l'heuristique DLIS, l'une 
étant meilleure que l'autre en fonction des expériences menées.



Comparaison des performances RENDU 3
====================================

L'objectif est ici de voir comment évoluent les temps d'exécutions du solveur 
avec l'apprentissage de clauses.
Par manque de temps, seules des résolutions du problème SAT ont été traitées 
(3SAT et 5SAT).
Pour chaque paramètres différents, 10 tests ont été faits, en changeant 
à chaque fois l'ensemble de clauses par un nouveau, généré aléatoirement.

Les fichiers des résultats expérimentaux se trouvent dans le répertoire
benchmarks_results/rendu3.
Ils comparent les résultats avec et sans apprentissage de clause, pour toutes les
heuristiques, avec et sans littéraux surveillés.
La courbe ci jointe (courbes.pdf) présente ces résultats.


3SAT
----

Pour les heuristiques MOMS et DLIS (qui sont les plus rapides), les temps
d'exécution avec ou sans apprentissage de clauses sont similaires.
On observe de plus grandes différences pour les heuristiques RAND et DUMB, pour
lesquels l'apprentissage de clause entraîne un gain remarquable.
Ces observations sont valables avec ou sans Watched Literals.


5SAT
----

De manière surprenante, l'apprentissage de clauses est ici moins performant, 
quelque soit l'heuristique, avec ou sans Watched Literals.
Ceci pourrait s'expliquer intuitivement par la taille des clauses apprises : 
celles ci sont en moyenne plus grande que pour 3SAT, ce qui diminue le gain
apporté par leur apprentissage. 
Au contraire, de plus grandes clauses entraîne un coût plus important pour les 
générer.


Conclusion
-----------

L'apprentissage de clause ne semble pas apporter de gain significatif pour les
heuristiques rapides. Dans certains cas il entraîne une perte de performances 
non négligeable.



Expériences RENDU 4
===================

Par manque de temps, ces expériences ont surtout permis de repérer puis corriger
des bugs.
Nous avons modifié le générateur (generator.ml) afin qu'il puisse générer des 
formules dont les termes sont des atomes des logiques de différence, égalité, 
ou congruence.
  * Génération d'une formule de profondeur 3, pour 5 variables, en logique de 
  différence :            
    ./generator -nvar 5 -depth 3 -difference
  * Génération d'une formule de profondeur 3, pour 5 variables, en logique de 
  congruence, avec des termes de profondeur 2 :            
    ./generator -nvar 5 -depth 3 -congruence 2
  * Génération d'une formule de profondeur 3, pour 5 variables, en logique  
  d'égalité :            
    ./generator -nvar 5 -depth 3 -congruence 0
    
Pour mesurer les performances de notre solveur de théorie, nous avons résolu
des formules de tailles croissantes (de 1 à 11), avec 5 variables.
Il aurait bien entendu été préférable de mener des réflexions plus poussées.
Par exemple, le nombre de variables influe peut être sur la complexité du problème.
Certains motifs de formules sont aussi susceptibles d'être plus difficiles que 
d'autres à résoudre.

Les résultats obtenus sont présents dans le dossier benchmarks_results/rendu4/.
Les courbes correspondantes sont visibles sur le fichier courbes.pdf.


Observation des courbes
-----------------------

On observe tout d'abord que l'heuristique RAND est la meilleure pour les formules
de logique d'égalité. Une explication est que cette heuristique favorise
les littéraux apparaissant souvent dans l'ensemble de clauses (choix équiprobable
pour chaque occurrence de littéral).

On observe également que l'apprentissage de clause ne produit pas de résultats 
catastrophiques. Au contraire, il y a parfois un gain (par exemple, la théorie
de l'égalité, heuristique RAND). C'est peut être simplement parce que l'on se
limite à des formules de petite taille.


Conclusion
----------

Nous n'obersons plus de bugs, et les résultats expérimentaux semblent raisonnables.
Des tests plus poussés sont nécessaires.
Il serait aussi judicieux d'implémenter un programme vérifiant les solutions
fournies par nos solveurs.
