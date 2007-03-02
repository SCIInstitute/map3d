#!/bin/sh
# Make the html version of the map3d manual
#
######################################################################
LATEX2HTMLOPTIONS="-split 4 -no_white -link 3 -bottom_navigation \
-html_version 4.0 -no_math -show_section_numbers -local_icons \
-long_titles 1 -prefix map3d- -image_type gif"
#LATEX2HTMLOPTIONS="-split 4 -no_white -link 3 -bottom_navigation \
#-html_version 4.0,math -no_math -show_section_numbers -local_icons \
#-long_titles 1 -prefix map3d- -image_type gif"

#-style ../../../Utilities/HTML/doc_styles.css

echo "Running command as latex2html ${LATEX2HTMLOPTIONS} manual"

latex2html ${LATEX2HTMLOPTIONS} manual

