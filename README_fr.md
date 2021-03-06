Utilisation
===========

Compilation
-----------

    $ mkdir build/
    $ cd build/
    $ cmake ..
    $ make

Exécution
---------

La compilation engendre trois exécutables `resol`, `colorie`, et `tseitin`.

Ils peuvent tous prendre les paramètres suivants (qui doivent être avant
les autres paramètres) :

* -verbose pour activer le mode verbeux
* -WL pour activer les littéraux surveillés
* -rand, -moms, ou -dlis (mutuellement exclusifs) pour activer les heuristiques
  du même nom
* -CL pour activer l’apprentissage de clauses
* -cl-interac pour activer le mode interactif

`colorie` prend également un entier qui est le nombre de couleur maximal à
utiliser.
`tseitin` peut également prendre deux paramètres :
* -print-interpretation qui indique comment le parseur a interprété la
  formule (revient à afficher la même formule, mais avec des parenthèses
  au lieu d’une priorité « implicite »
* -print-sat qui affiche le problème SAT engendré par cette formule.

Enfin, ils peuvent tous prendre un nom de fichier comme dernier paramètre,
qui sert d’entrée (si le nom de fichier n’est pas donné, l’entrée standard
est utilisée).

Les fichiers en entrée de `resol` et `colorie` sont respectivement aux
formats `cnf` et `edge` du DIMACS. Celui de `tseitin` est une formule
telle que définie dans le cours.


Remarque : vu que la sortie de `colorie` est le format d’entrée de `dot`,
on peut utiliser cette commande pour afficher directement le graphe, avec
la commande `display` d’ImageMagick (ou n’importe quel autre programme
lisant du PNG sur son entrée standard) :
 ./colorie 2 example.col | dot -Tpng | display


Tests
-----

Les tests unitaires ne sont compilés *que* si la bibliothèque CppUnit est
installée (libcppunit-dev sous Debian/Ubuntu).
Ils sont compilés en même temps que les autres exécutables, et sont lancés
par l’exécutable `testrunner`.


Implémentation
==============

Organisation du code
--------------------

Les structures de données sont toutes implémentées dans une paire de
fichiers situés dans src/structures/.
La majorité des opérations effectuables sur ces structures sont
implémentées directement en tant que méthodes de ces objets, ce qui
permet de réduire la taille de l’implémentation de DPLL lui-même et
de s’affranchir complètement de l’implémentation des structures
de données.

Dans le but d’éviter la duplication de code, chaque exécutable
engendré (excepté testrunner) correspond à un des fichiers
de src/entry_points/.

Les formats imposés pour les fichiers CNF et EDGE étant très simples,
ils peuvent être analysés par un simple automate fini,
implémenté dans src/parsers/{abstract,sat,graph}.cpp.

Le format pour les formules étant plus complexe, il faut au moins un
automate à pile pour le reconnaître, ce qui est fait en utilisant
Flex et Bison, dans src/parsers/extended_formula.*.


Structures de données et choix d’implémentation
-----------------------------------------------

* Clauses :
    * Ensemble de littéraux (unordered_set, basés sur une table de
      hachage, afin d'avoir un accès en temps constant en moyenne).
    * Pointeur vers l'affectation courante de la formule afin de
      pouvoir tester si un littéral est à vrai/faux/indéterminé.
* Affectations :
    * Tableau d'entier, la i-ème case représentant l'état du littéral i.
* Formules :
    * Tableau de clauses.
    * Pointeur sur l'affectation courante.
    * Une pile de couples (entier,booléen). L'entier au sommet est le
      dernier littéral affecté à vrai. Si le booléen associé est true,
      alors c'était une déduction, sinon c'était un pari.
    * Un ensemble de littéraux, qui est l'ensemble des contraintes
      engendrées lors d'une affectation (tous les littéraux de cet ensemble
      doivent être affectés à vrai, et si x et -x sont présents alors
      il y a un conflit).
* Formules étendues :
    * Arbre (évidemment)
    * Éventuellement, si un nœud est étiqueté par =>, une « traduction » en
      un sous-arbre en substituant (A ∨ ¬B) à (A => B). (Pour éviter de la
      recalculer plusieurs fois si on en a besoin)
    * Lors de l’analyse des formules étendues (c’est-à-dire dans
      src/parsers/extended_formula.y), la priorité suivante est donnée
      aux opérateurs : NOT > AND > OR > IMPLIES
* Déductions :
    * contient (sous forme de vecteurs), pour chaque littéral, la
      profondeur de déduction à laquelle il a été déduit, et l'indice de la
      clause ayant permis sa déduction
    * contient aussi, seulement lorsque l'option cl-interac est active, deux
      vecteurs d'ensembles, formant une double correspondance entre littéraux 
      déduits, et les littéraux ayant permis la déduction (en valeurs absolues)
* Preuves :
    * La classe CLProof contient un arbre de dérivation ayant permis
      d’engendrer une clause apprise. Cet arbre étend en réalité un peigne,
      on le stocke sous forme d’un vecteur de paires de clauses.


Réduction d’un problème de coloriage en une formule
---------------------------------------------------

Pour un problème de coloriage à k couleurs, on crée ceil(log2(k)) variables
par nœud, correspondant à un codage en binaire de la coloration de ce nœud.

On crée alors des formules :
* Pour chaque nœud, s’assurer que sa couleur est strictement inférieure à k
* Pour chaque arrête, s’assurer que ses deux extrémités sont de couleur
  différente (revient à vérifier qu’au moins un bit diffère entre les deux)

Il ne reste alors plus qu’à effectuer une transformation de Tseitin sur
la formule, résoudre le problème SAT, et retrouver l’affectation des
variables de la formule à partir des « littéraux de SAT ».


Modifications nécessaires pour le rendu 3
-----------------------------------------

Pour ajouter des clauses en cours de route :
    * Il suffit d’ajouter des éléments à la fin du vecteur
    contenant toutes les clauses
    * Pour les clauses de taille 1 déduites, nous avons choisi de ne pas les 
    apprendre (ça aurait posé des problèmes avec WL), mais de backtracker au
    niveau 0 et déduire l'unique littéral de la clause (cela produit quelque 
    chose d'équivalent à l'apprentissage dans ce cas)
    * Lorsque l'on apprend une clause et que l'on utilise l'heuristique WL, on
    choisit comme littéraux surveillés le premier UIP (seul littéral restant du
    niveau courant), ainsi qu'un autre littéral, de niveau de déduction maximal.
    Cela permet que les choses "se passent bien" lors des futurs backtracks. En 
    effet, après le premier backtrack, le 1UIP sera déduit à vrai. Si un autre
    backtrack survient par la suite, alors les deux littéraux surveillés seront 
    d'affectation inconnue.
    Si l'on avait choisit de surveiller un littéral de niveau inférieur, après 
    ce deuxième backtrack on aurait eu comme littéraux surveillés un littéral 
    faux, et un littéral inconnu, alors que la clause contiendrai au moins deux 
    littéraux inconnu (on ne respecterai donc plus l'invariant des WL).

Garder une trace de la raison pour laquelle un littéral a été déduit :
    cf ci-dessus (la structure « Déductions »)



Heuristiques
=============

Quatre heuristiques sont implémentées pour le moment.
* DUMB : on parie sur les littéraux 1,2,...,n dans cet ordre.
* RAND : on choisit avec une probabilité uniforme un littéral parmi toutes les
         instances de littéraux non affectés dans des clauses non satisfaites.
         Ainsi, un littéral apparaissant plus souvent a plus de chances d'être
         sélectionné.
         Un littéral présent seulement dans des clauses satisfaites (ou
         totalement absent) ne sera jamais sélectionné.
* MOMS : on parie sur le littéral le plus présent, parmi les clauses non
         satisfaites de taille minimale.
         La taille d'une clause est ici son nombre de littéraux non satisfaits.
* DLIS : pour un littéral x non affecté dans une clause c non satisfaite, on
         définit pt_c(x) = 2^(-s) avec s la taille de la clause (définie ci
         dessus).
         On définit ensuite Pt(x) comme la somme des pt_c(x), pour toute
         clause c non satisfaite.
         On pari alors sur le littéral de somme Pt(x) maximale.


Exemple pédagogique
===================

On prend comme exemple le fichier examples/simple.cnf (pour l’exécuter :
« ./resol -verbose -CL -cl-interact ../examples/simple.cnf »)

a) On commence par parier sur 1 et 2.
b) Grâce a) et à la seconde clause, on en déduit -4.
c) Grâce à c) et à la troisième clause, on déduit 5.
(on ignore les clauses quatre et cinq, car elles sont satisfaites)
d) Grâce à c) et à la clause six (resp. sept) on déduit 6 (resp. sept)
e) Grâce à d) et à la huitième clause, on déduit 8.
f) On détecte un conflit pour la neuvième clause.

Le graphe engendré à ce niveau traduit bien ce qu’il vient de se passer
(avec le niveau de « dédoublement » correspondant à d) )

On déduit alors la clause « -5 » (cf le fichier de preuve), ce qui vient
du fait que les deux dernières clauses impliquent que l’on a -6 ou -7,
ce qui implique que l’on ait -5.
Comme expliqué plus haut, on n'ajoute pas cette clause, car elle est de taille 1.
On fait donc un backtrack au niveau 0, et on déduit -5.
Puis on en déduit 4 vu la troisième clause, puis on déduit -6 puisqu’il apparaît 
uniquement avec cette polarité, etc…


Résolution de théories
======================

Principe
--------

Nous implémentons les solveurs de théorie sous forme d’« assistants » à
DPLL : ils fournissent des informations supplémentaires à DPLL, par exemple
le fait qu’une valuation soit cohérente vis-à-vis de la signification réelle
des atomes (l’assistant qui dit que la valuation est toujours cohérente
correspond à la résolution du problème SAT classiques).

Pour cela, la méthode « on_flip » de l’assistant est appelée à chaque
modification de la polarité (vrai/faux/inconnue) d’une variable, et celle-ci
vérifie que l’état est cohérent, et en informe DPLL. Si nécessaire,
elle peut également ajouter des clauses au problème pour empêcher DPLL
de faire une « erreur » similaire dans le futur.

Lors de l'apprentissage d'une clause par la théorie, on reprend des idées de 
l'apprentissage d'une clause classique.
Si l'état initial est consistant, alors une inconsistance ne peut être 
introduite que si l'on a une clause représentant xi-xj>a1 et une clause 
représentant xj-xi>a2 (cas de la théorie de différences, mais l'idée est la même
pour la théorie de l'égalité). Donc toute clause apprise est de taille au moins 2.
On définit alors facilement les littéraux surveillés comme étant le littéral
qui vient d'être affecté, et un littéral de niveau de déduction maximal (pour les
mêmes raisons que l'apprentissage de clauses classique... voir ci dessus, 
"rendu 3", sauf que le 1UIP que l'on avait est ici le littéral qui a généré le 
conflit, donc le dernier littéral affecté).
On effectue ensuite un backtrack au niveau de déduction maximal (sans compter
le dernier littéral affecté).
Si ce niveau est égal au niveau courant, il n'y a pas de backtrack. La clause que
l'on vient d'ajouter est donc fausse : on laisse alors la suite de l'algorithme
s'en occuper (backtrack classique).
Sinon, ce niveau est strictement inférieur au niveau courant. Donc après le 
backtrack, la clause apprise ne contiendra qu'un littéral inconnu, tous les autres
étant faux. On peut donc déduire immédiatement ce littéral comme étant vrai (et 
la clause ayant permis cette déduction est la clause que l'on vient d'apprendre).

Théorie de différences
----------------------

L’assistant pour la théorie de différences maintient un multigraphe orienté
et pondéré, où chaque arête correspond à un atome.

Comme un état est cohérent si et seulement si il n’y a pas de cycle de poids
négatif.
Donc, pour tester si l’état reste cohérent, il suffit, lorsque l’on ajoute
une arête uv de poids w, de vérifier qu’il n’y a pas de chemin de v à u de
poids strictement inférieur à -w (ce qui revient à chercher un chemin de
poids minimal).

Théorie d’égalités
------------------

On utilise une forme d’union-find, mais qui gère la « désunion » (nécessitant
donc de ne pas compresser les chemins) et qui mémorise l’union de deux éléments
déjà dans le même ensemble, c’est-à-dire que par exemple, si on dit deux fois
que x1 et x2 sont dans la même classe d’équivalence, si on ne fait qu’une seule
désunion, il seront toujours dans la même.

L’ajout de clause se fait lorsque l’on rajoute un atome xi!=xj si xi et xj
sont dans la même classe d’équivalence, ou un atome xi=xj et que cela
place dans la même classe d’équivalence deux variables censées être
différents.
Dans ce cas, la clause ajoutée contient tous les littéraux intervenant
dans le fait que xi et xj soient dans la même classe d’équivalence,
ainsi que le littéral correspondant à la diségalité engendrant le
conflit.
Elle n’est certes pas minimale dans le cas général, mais reste de taille
tout à fait raisonnable.

À la fin de la résolution d’un problème d’égalité, on affiche aussi une
affectation des variables. (Il suffit d’afficher l’indice du représentant
de la classe d’équivalence de chacune des variables.)




Répartition du travail
======================

Fonctionnalités
---------------

Tom :
* Structures du rendu 1
* Ajout des littéraux surveillés à l’algorithme de DPLL
* Heuristiques
* Générateur de données, tests de performances, moulinettes
* Apprentissage de clause avec littéraux surveillés
* Intégration des assistants avec WL et/ou CL.
* Important travail de débogage pour le rendu 4.

Valentin :
* Parseur de CNF et EDGE
* Algorithme de DPLL (sans les littéraux surveillés)
* Classe et parseur pour les formules étendues (+ pour SMT) ; réduction de Tseitin
* Réduction problème de graphe -> formule étendue
* Graphe de conflits
* Apprentissage de clauses sans les littéraux surveillés, preuves
* « Assistants » de théorie de différence et d’égalité + AdjGraph & UnionFind

Fichiers
--------

(résumé de ce qu’affiche « git shortlog » pour chacun des fichiers)

benchmarks/ :
* Tom

CMakeLists.txt :
* Valentin

examples/ :
* Internet (sauf simple.cnf)

EXPERIENCES :
* Tom

README :
* Valentin et Tom (selon les parties concernées)

src/entry_points/ :
* Principalement Valentin (structure, …)
* Corrections de bugs par Tom

src/parsers/ :
* Valentin

src/solvers/dpll.* :
* Principalement par Tom (algo de DPLL et littéraux surveillés)
* quelques éléments du rendu 3 par Valentin

src/solvers/graphcolor.* :
* Valentin

src/structures/{affectation,clause,formula}.* :
* Tom

src/structures/{CL_proof,color_affectation,extended_formula,graph}.* :
* Valentin

src/structures/deductions.* :
* Première écriture par Valentin
* Réécriture partielle par Tom pour inclure les objets Deductions
  dans les Formula.

tests/structures_tests.* :
* Tom et Valentin (au fur et à mesure de l’avancement)

tests/{graphsolver,graphparser,satparser}_tests.* :
* Valentin

tests/{heuristics,dpll}_tests.* :
* Tom
