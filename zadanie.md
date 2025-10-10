# Zadanie Zaliczeniowe

-----

## Ogólne Sformułowanie Zadania

Należy rozszerzyć funkcjonalność [**emulatora bazy danych
Bigtable**](https://github.com/Unoperate/bigtable-emulator/tree/main) o jedną z
wybranych funkcjonalności:

* **Persystencja**
* **Zapytania SQL**

-----

## Kontekst

Google stworzył **Bigtable** jako system zarządzania bazą danych (SZBD)
wykorzystywany do indeksowania Internetu. Oryginalny artykuł opisujący jego
działanie dostępny jest
[**tutaj**](https://static.googleusercontent.com/media/research.google.com/en//archive/bigtable-osdi06.pdf).

Od 2016 roku baza ta jest dostępna dla użytkowników zewnętrznych pod nazwą
[**Cloud Bigtable**](https://cloud.google.com/bigtable?hl=en).

Na zamówienie Google powstaje nowy [**emulator tej bazy
danych**](https://github.com/Unoperate/bigtable-emulator/tree/main). Obecnie
implementuje on większość [**API Cloud
Bigtable**](https://github.com/googleapis/googleapis/tree/master/google/bigtable),
ale brakuje mu m.in. wsparcia dla zapytań SQL oraz **persystencji** (tzn.
restart bazy danych powoduje utratę zapisanych danych).

Zadanie polega na zaimplementowaniu jednej z tych funkcjonalności.

-----

## Ustalenia Ogólne
* Praca odbywa się w **zespołach, maksymalnie 3-osobowych**. Proszę o przesłanie
  składów na maila **do 16.10.2025**.
* Cały zespół otrzyma taką samą ocenę.
* Każdy zespół powinien utworzyć własnego **forka** [repozytorium
  `bigtable-emulator`](https://www.google.com/search?q=%5Bhttps://github.com/Unoperate/bigtable-emulator/tree/main%5D\(https://github.com/Unoperate/bigtable-emulator/tree/main\)).
* Rozwiązanie należy dostarczyć jako **Pull Request (PR)** do głównego
  repozytorium.
* Jeśli zespół będzie chciał zaprezentować częściowe rozwiązanie, należy to
  zrobić jako PR do **własnego forka**.
* Rozwiązania muszą być pokryte **unit testami**.
* **Formater i linter** nie mogą zgłaszać żadnych uwag.
* Należy użyć obecnego build chaina ([**Bazel**](https://bazel.build)).
* Stosujemy się do [**Google style
  guide**](https://google.github.io/styleguide/cppguide.html).
* Ocenie podlegać będą:
  * **Kompletność i poprawność**
  * **Wydajność** (oceniana głównie pod kątem *oczywistych problemów
    architektonicznych* lub *niedbałego kodu*)
  * **Jakość kodu** (czytelność, spójność z resztą projektu, idiomatyczność,
    dobra struktura)

-----

## Wariant I: Dodawanie Persystencji

* Wymagane jest użycie [**RocksDB**](https://rocksdb.org/).
* Pozostawienie implementacji *in-memory* nie jest wymagane, ale jest
  dopuszczalne.
* "Column families" z emulatora powinny być zmapowane na "column families" w
  RocksDB.

-----

## Wariant II: Dodawanie Zapytań SQL

* Wymagane jest użycie [**ZetaSQL**](https://github.com/google/zetasql)
  (zarówno analizatora, jak i silnika).
* Efektem jest wsparcie dla zapytań SQL w API Bigtable, w metodzie
  `ExecuteQuery` ([zobacz
  definicję](https://github.com/googleapis/googleapis/blob/0509f5df22c35606062249a2ce2b00877ab2f445/google/bigtable/v2/bigtable.proto#L316a)).

-----

## Rekomendacje

* Sugeruję **rozpocząć prace wcześnie**, aby upewnić się, że nie napotkacie
  niespodzianek.
* Sugeruję **często weryfikować przyjęty kierunek** – będziemy w stanie wspólnie
  wyłapać i wskazać problemy, które niepoprawione mogłyby skutkować niższą oceną.

-----

## Zasady Współpracy

* Wysłane **Pull Requesty** w trakcie semestru ocenię ja lub wyznaczona przeze
  mnie osoba z firmy – zajmie to **nie dłużej niż tydzień**.
* Odpowiedzi na maile z pytaniami będą udzielane w miarę możliwości (*best
  effort*).
* **Konsultacje na laboratoriach** – od połowy semestru zakładam, że będą one
  jedyną formą pracy na zajęciach.
* **Termin składania rozwiązań** jeszcze potwierdzę, ale najprawdopodobniej
  będzie to **koniec sesji**.

-----

## Wskazówki

* Do interakcji z emulatorem można użyć [**narzędzia CLI**
  (`cbt`)](https://www.google.com/search?q=%5Bhttps://cloud.google.com/bigtable/docs/cbt-overview%5D\(https://cloud.google.com/bigtable/docs/cbt-overview\))
  lub bibliotek klienckich.
* ZetaSQL jest używane w emulatorze Spannera, co może stanowić cenne źródło
  inspiracji.
