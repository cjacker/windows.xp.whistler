;
; status.S
;
; Bandit status bar messages
;

IDS	5500, 6000
SEG	STATUS

MenuStatusFile		,"Hallitsee tapaamiskirjoja tai lopettaa Schedule+:n."
MenuStatusTurnOffAlarms	,"Kytkee muistutukset pois."
MenuStatusConnectSvr		,"Yhdist�� palvelimelle."

MenuStatusExport		,"Vie ty�j�rjestystiedot."	
MenuStatusImport		,"Tuo ty�j�rjestystiedot."	
MenuStatusCreateArchive	,"Luo arkistotiedoston."
MenuStatusOpenArchive		,"Avaa arkistotiedoston."
MenuStatusPrint		,"Tulostaa ty�j�rjestystiedot."	
MenuStatusPrintSetup		,"Muuttaa kirjoittimen asetuksia."
MenuStatusExit		,"Lopettaa Schedule+:n k�yt�n."	
MenuStatusTurnOnAlarms	,"K�ynnist�� muistutukset-ohjelman"
MenuStatusDisconnectSvr	,"Katkaisee yhteyden palvelimeen."
MenuStatusMoveLocalFile	,"Siirt�� paikallisen ty�j�rjestystiedoston."

MenuStatusEdit		,"Kumoaa, poistaa, kopioi, liitt��, etsii tai siirtyy valittuun p�iv��n."	
MenuStatusUndo		,"Kumoaa/toistaa viimeksi suoritetun toiminnon."	
MenuStatusCut			,"Leikkaa valinnan."	
MenuStatusCopy		,"Kopioi valinnan ja sijoittaa sen Leikep�yd�lle."	
MenuStatusPaste		,"Lis�� Leikep�yd�n sis�ll�n lis�yskohtaan."	
MenuStatusEditAppt		,"N�ytt�� tapaamisen tiedot muokkaamista varten."
MenuStatusCopyAppt		,"Kopioi valitun tapaamisen ja sijoittaa sen Leikep�yd�lle."
MenuStatusMoveAppt		,"Siirt�� valitun tapaamisen."
MenuStatusDeleteAppt		,"Poistaa valitun tapaamisen."
MenuStatusFind		,"Etsii teksti� tapaamisista tai p�ivitt�isist� huomautuksista."	
MenuStatusGoToDate		,"Siirtyy m��ritettyyn tapaamiskirjan p�iv��n."
MenuStatusGoToWeek		,"Siirtyy m��ritettyyn tapaamiskirjan tai suunnittelun viikkoon."
MenuStatusEditTask		,"N�ytt�� teht�v�n tiedot muokkaamista varten."
MenuStatusDeleteTask		,"Poistaa valitun teht�v�n."

MenuStatusAppointment		,"Luo tai muuttaa tapaamisia, valitsee kokousajan automaattisesti tai l�hett�� postin uudelleen."
MenuStatusNewAppt		,"Luo uuden tapaamisen."	
MenuStatusAutoPick		,"Valitsee seuraavan ajan, jolloin kaikki osallistujat ovat vapaina valitun ajanjakson. "
MenuStatusSetTentative		,"Muuttaa valitun tapaamisen alustavaksi."
MenuStatusRemoveTentative	,"Vahvistaa valitun alustavan tapaamisen."
MenuStatusSetPrivate		,"Muuttaa valitun tapaamisen yksityiseksi."
MenuStatusRemovePrivate		,"Muuttaa valitun oman tapaamisen julkiseksi."
MenuStatusSetReminder		,"Asettaa muistutuksen valituille tapaamisille."
MenuStatusRemoveReminder	,"Poistaa muistutuksen valituilta tapaamisilta."
MenuStatusResend		,"L�hett�� osallistujille muistutuksen tai uuden kokouspyynn�n."	
MenuStatusNewRecur		,"Luo uuden toistuvan tapaamisen."
MenuStatusRecurAppt		,"N�ytt�� toistuvien tapaamisten luettelon."
MenuStatusOpenApptBook	,"N�ytt�� m��ritetyn k�ytt�j�n tapaamiskirjan."

MenuStatusToDo		,"Luo uusia teht�vi� tai projekteja, muuttaa toistuvia teht�vi� tai lajittelee teht�v�t."	
MenuStatusNewTask		,"Luo uuden teht�v�n."	
MenuStatusNewProject		,"Luo uuden projektin."
MenuStatusNewRecurTask	,"Luo uuden toistuvan teht�v�n."
MenuStatusRecurTask		,"N�ytt�� toistuvien teht�vien luettelon."
MenuStatusViewByProject	,"N�ytt�� teht�v�t ja projektit teht�v�luettelossa."
MenuStatusViewNotProject	,"N�ytt�� vain teht�v�t teht�v�luettelossa."
MenuStatusShowAllTasks	,"N�ytt�� aktiiviset ja passiiviset teht�v�t."
MenuStatusTaskSortPri	, "Lajittelee teht�v�t prioriteetin mukaan."
MenuStatusTaskSortDue	, "Lajittelee teht�v�t m��r�p�iv�n mukaan."
MenuStatusTaskSortDesc	, "Lajittelee teht�v�t kuvausten perusteella aakkosj�rjestykseen."
MenuStatusShowActiveTasks	,"N�ytt�� vain aktiiviset teht�v�t."

MenuStatusOptions		,"Muuttaa salasanaa, asettaa k�ytt�oikeudet tai muuttaa Schedule+:n asetuksia."	
MenuStatusGeneralOptions	,"Asettaa ty�ajan, k�ynnistysasetukset ja ohjelman yleiset asetukset."
MenuStatusDisplayOptions	,"Asettaa ty�j�rjestyksen v�rit ja fonttikoon."
MenuStatusChgPassword		,"Muuttaa Mail- ja Schedule+-salasanasi."
MenuStatusChgAccessPriv	,"Asettaa muiden k�ytt�jien k�ytt�oikeudet ty�j�rjestyksellesi."
MenuStatusShowTime		,""
MenuStatusHideTime		,"Poistaa tilarivin."

MenuStatusWindowMenu		,"J�rjest�� ikkunat uudelleen tai aktivoi m��ritetyn ikkunan."
MenuStatusCascade		,"J�rjest�� avoimet ikkunat limitt�in."
MenuStatusTile		,"J�rjest�� avoimet ikkunat vierekk�in."
MenuStatusArrangeIcons	,"J�rjest�� pienennettyjen ikkunoiden kuvakkeet."
MenuStatusMDIWindow		,"Vaihtaa m��ritettyyn ikkunaan."

MenuStatusHelp		,"Antaa tietoja ja ohjeita Schedule+:n k�yt�st�."	
MenuStatusHelpContents	,"N�ytt�� Ohjeen sis�llysluettelon."
MenuStatusHelpIndex		,"N�ytt�� aakkosellisen luettelon ohjeen termeist� ja k�sitteist�."
MenuStatusHelpTutorial	,"K�ynnist�� Schedule+:n opetusohjelman."
MenuStatusAbout		,"N�ytt�� tietoja ohjelmasta,  Schedule+:n versionumeron, k�ytt�oikeus- ja tekij�noikeustiedot."	

MenuSystem		,"Siirt�� tai sulkee ikkunan tai muuttaa sen kokoa."
MenuSystemSize		,"Muuttaa ikkunan koon."
MenuSystemMove		,"Muuttaa ikkunan sijainnin n�yt�ss�."
MenuSystemMinimize	,"Pienent�� ikkunan kuvakkeeksi."
MenuSystemMaximize	,"Laajentaa ikkunan enimm�iskokoonsa."
MenuSystemNext		,"Aktivoi seuraavan ikkunan."
MenuSystemClose		,"Sulkee aktiivisen ikkunan."
MenuSystemRestore	,"Palauttaa ikkunan normaaliin kokoonsa."
MenuSystemSwitchTo	,"Vaihtaa toiseen ikkunaan."

SEG

StatusAllTasks,		,"Kaikki teht�v�t"
StatusActiveTasks, 	,"Aktiiviset teht�v�t"
StatusRange,		,"%s - %s"
StatusRangeFull,	,"%s, %s - %s, %s"

; BUG: Move up NEW
SEG	STATUS
MenuStatusExitSignOut	,"Lopettaa kaikki ty�ryhm�sovellukset."	
MenuStatusHelpSearch	,"Etsii ohjeen aiheen perusteella"
MenuStatusHelpOnHelp	,"N�ytt�� tietoja Ohjeen k�yt�st�"

