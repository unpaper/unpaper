
# generated via xslt
UNPAPER=/home/jgulden/workspace/unpaper/unpaper
PNGTOPNM=/usr/bin/pngtopnm
PNMTOPNG=/usr/bin/pnmtopng
TMP=/tmp
        
# ----------------------------------------------------------------------------    
# [A1] Single-Page Template Layout, Black+White, Full Processing
$PNGTOPNM ../img/test001.png > test001.pbm
        

$UNPAPER --version > ../log/testA1.txt
$UNPAPER -v --overwrite test001.pbm resultA1.pbm >> ../log/testA1.txt
cat ../log/testA1.txt


$PNMTOPNG resultA1.pbm > ../img-results/resultA1.png
rm resultA1.pbm
rm test001.pbm
        
echo Done: [A1] Single-Page Template Layout, Black+White, Full Processing.
echo
            
    
# ----------------------------------------------------------------------------    
# [B1] Combined Color/Gray, No Processing
$PNGTOPNM ../img/test003.png > test003.ppm
$PNGTOPNM ../img/test004.png > test004.pgm
        

$UNPAPER --version > ../log/testB1.txt
$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence test003.ppm test004.pgm --output-file-sequence resultB1.ppm >> ../log/testB1.txt
cat ../log/testB1.txt


$PNMTOPNG resultB1.ppm > ../img-results/resultB1.png
rm resultB1.ppm
rm test003.ppm
rm test004.pgm
        
echo Done: [B1] Combined Color/Gray, No Processing.
echo
            
    
# ----------------------------------------------------------------------------    
# [B2] Combined Color/Black+White, No Processing
$PNGTOPNM ../img/test003.png > test003.ppm
$PNGTOPNM ../img/test005.png > test005.pbm
        

$UNPAPER --version > ../log/testB2.txt
$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence test003.ppm test005.pbm --output-file-sequence resultB2.ppm >> ../log/testB2.txt
cat ../log/testB2.txt


$PNMTOPNG resultB2.ppm > ../img-results/resultB2.png
rm resultB2.ppm
rm test003.ppm
rm test005.pbm
        
echo Done: [B2] Combined Color/Black+White, No Processing.
echo
            
    
# ----------------------------------------------------------------------------    
# [B3] Combined Gray/Black+White, No Processing
$PNGTOPNM ../img/test004.png > test004.pgm
$PNGTOPNM ../img/test005.png > test005.pbm
        

$UNPAPER --version > ../log/testB3.txt
$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence test004.pgm test005.pbm --output-file-sequence resultB3.pgm >> ../log/testB3.txt
cat ../log/testB3.txt


$PNMTOPNG resultB3.pgm > ../img-results/resultB3.png
rm resultB3.pgm
rm test004.pgm
rm test005.pbm
        
echo Done: [B3] Combined Gray/Black+White, No Processing.
echo
            
    
# ----------------------------------------------------------------------------    
# [C1] Black sheet background color
$PNGTOPNM ../img/test002.png > test002.pbm
        

$UNPAPER --version > ../log/testC1.txt
$UNPAPER -v --overwrite -n --sheet-size a4 --sheet-background black test002.pbm resultC1.pbm >> ../log/testC1.txt
cat ../log/testC1.txt


$PNMTOPNG resultC1.pbm > ../img-results/resultC1.png
rm resultC1.pbm
rm test002.pbm
        
echo Done: [C1] Black sheet background color.
echo
            
    
# ----------------------------------------------------------------------------    
# [C2] Explicit shifting.
$PNGTOPNM ../img/test002.png > test002.pbm
        

$UNPAPER --version > ../log/testC2.txt
$UNPAPER -v --overwrite -n --sheet-size a4 --pre-shift -5cm,9cm test002.pbm resultC2.pbm >> ../log/testC2.txt
cat ../log/testC2.txt


$PNMTOPNG resultC2.pbm > ../img-results/resultC2.png
rm resultC2.pbm
rm test002.pbm
        
echo Done: [C2] Explicit shifting..
echo
            
    
# ----------------------------------------------------------------------------    
# [D1] Crop to sheet size
$PNGTOPNM ../img/test003.png > test003.ppm
        

$UNPAPER --version > ../log/testD1.txt
$UNPAPER -v --overwrite -n --sheet-size 20cm,10cm test003.ppm resultD1.ppm >> ../log/testD1.txt
cat ../log/testD1.txt


$PNMTOPNG resultD1.ppm > ../img-results/resultD1.png
rm resultD1.ppm
rm test003.ppm
        
echo Done: [D1] Crop to sheet size.
echo
            
    
# ----------------------------------------------------------------------------    
# [D2] Fit to sheet size
$PNGTOPNM ../img/test003.png > test003.ppm
        

$UNPAPER --version > ../log/testD2.txt
$UNPAPER -v --overwrite -n --size 20cm,10cm test003.ppm resultD2.ppm >> ../log/testD2.txt
cat ../log/testD2.txt


$PNMTOPNG resultD2.ppm > ../img-results/resultD2.png
rm resultD2.ppm
rm test003.ppm
        
echo Done: [D2] Fit to sheet size.
echo
            
    
# ----------------------------------------------------------------------------    
# [D3] Stretch to sheet size
$PNGTOPNM ../img/test003.png > test003.ppm
        

$UNPAPER --version > ../log/testD3.txt
$UNPAPER -v --overwrite -n --stretch 20cm,10cm test003.ppm resultD3.ppm >> ../log/testD3.txt
cat ../log/testD3.txt


$PNMTOPNG resultD3.ppm > ../img-results/resultD3.png
rm resultD3.ppm
rm test003.ppm
        
echo Done: [D3] Stretch to sheet size.
echo
            
    
# ----------------------------------------------------------------------------    
# [E] Splitting 2-page layout into seperate output pages
$PNGTOPNM ../img/testE001.png > testE001.pbm
$PNGTOPNM ../img/testE002.png > testE002.pbm
$PNGTOPNM ../img/testE003.png > testE003.pbm
        

$UNPAPER --version > ../log/testE.txt
$UNPAPER -v --overwrite --layout double --output-pages 2 testE%03d.pbm resultE%03d.pbm >> ../log/testE.txt
cat ../log/testE.txt


$PNMTOPNG resultE001.pbm > ../img-results/resultE001.png
rm resultE001.pbm
$PNMTOPNG resultE002.pbm > ../img-results/resultE002.png
rm resultE002.pbm
$PNMTOPNG resultE003.pbm > ../img-results/resultE003.png
rm resultE003.pbm
$PNMTOPNG resultE004.pbm > ../img-results/resultE004.png
rm resultE004.pbm
$PNMTOPNG resultE005.pbm > ../img-results/resultE005.png
rm resultE005.pbm
$PNMTOPNG resultE006.pbm > ../img-results/resultE006.png
rm resultE006.pbm
rm testE001.pbm
rm testE002.pbm
rm testE003.pbm
        
echo Done: [E] Splitting 2-page layout into seperate output pages.
echo
            
    