set terminal jpeg size 800,600
set output '850MHz_propagation_models.jpg'
set title '850MHz path loss by propagation model - CloudRF.com'
set key right bottom box
set style line 1
set grid
set xlabel 'Distance KM'
set ylabel 'Path Loss dB'
plot 'FSPL' with lines, 'Hata.1' with lines, 'COST231.1' with lines,'ECC33.1' with lines,'Ericsson9999.1' with lines,'SUI.1' with lines,'Egli.VHF-UHF' with lines