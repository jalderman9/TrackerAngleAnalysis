#!/usr/bin/Rscript

library(tidyr)
library(ggplot2)

angleswsa <- read.csv('AngleSummary_All.csv', header=TRUE)
head(angleswsa)

ggplot(data=angleswsa, aes(x=ANGLE_BIN, y=PERCENT_OF_TIME, color=LOCATION)) +
  geom_line() +
  coord_cartesian(ylim=c(0,60)) +
  labs(x='Tracker Angle (deg)', y="Percent of Time at Angle") +
  labs(title="Percent of Time at Tracker Angle - With Backtracking") +
  scale_x_continuous(breaks=c(0,10,20,30,40,50,60,70,80,90)) +
  theme( plot.title = element_text(family="Garamond", size=18, margin=(unit(c(0.2,1,0.6,1),"cm"))) ) +
  theme( axis.title.x = element_text(family="Garamond", size=13, margin = (unit(c(0.8,0.8,0.8,0.8), "cm")) ) ) +
  theme( axis.title.y = element_text(family="Garamond", size=13, margin = (unit(c(0.8,0.8,0.8,0.8), "cm")) ) ) +
  theme( plot.margin = unit(c(0.2,2.4,0.2,1), "cm") ) +
  guides(color=guide_legend(title="Location"))

ggplot(data=angleswsa, aes(x=ANGLE_BIN, y=PERCENT_OF_TIME, color=LOCATION)) +
  geom_line() +
  coord_cartesian(ylim=c(0,10.0)) +
  labs(x='Tracker Angle (deg)', y="Percent of Time at Angle") +
  labs(title="Percent of Time at Tracker Angle - With Backtracking") +
  scale_x_continuous(breaks=c(0,10,20,30,40,50,60,70,80,90)) +
  theme( plot.title = element_text(family="Garamond", size=18, margin=(unit(c(0.2,1,0.6,1),"cm"))) ) +
  theme( axis.title.x = element_text(family="Garamond", size=13, margin = (unit(c(0.8,0.8,0.8,0.8), "cm")) ) ) +
  theme( axis.title.y = element_text(family="Garamond", size=13, margin = (unit(c(0.8,0.8,0.8,0.8), "cm")) ) ) +
  theme( plot.margin = unit(c(0.2,2.4,0.2,1), "cm") ) +
  guides(color=guide_legend(title="Location"))
