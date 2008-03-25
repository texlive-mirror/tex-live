#!/usr/bin/env perl

$language = "Greek";
$prefix = "el";
$script = "greek";

$alphabet = [
['�',    ['�', '�'],   ['�','�']],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�'],   ['�','�']],
['�',    ['�', '�']],
          [],
['�',    ['�', '�'],   ['�','�']],
['�',    ['�', '�']],
['�',    ['�', '�'],   ['�','�'],   ['�', '�'],   ['�']],
          [],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�'],   ['�','�']],
['�',    ['�', '�']],
          [],
['�',    ['�', '�']],
['�',    ['�', '�'], ['�']],
['�',    ['�', '�']],
['�',    ['�', '�'],   ['�','�'],   ['�', '�'],   ['�']],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�']],
['�',    ['�', '�'],   ['�', '�']]
];

@special = ('?', '!', '.', ';', '�', 'letters', '-', '\'', '\\/');

# first lower or upper case?

$sortcase = "Aa";
#$sortcase = "aA";

#@letter_group_names = ('�','�','�','�','�','�','�','�','�','�','�','�',
#'�','�','�','�','�','�','�','�','�','�','�','�');

do 'make-rules.pl';
