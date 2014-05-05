/*
*********************************************************************
                                                                   
INPFILE.C -- Save Input Function for EPANET Program                
                                                                    
VERSION:    2.00                                               
DATE:       5/8/00
            3/1/01
            11/19/01                                          
            6/24/02
            8/15/07    (2.00.11)
AUTHOR:     L. Rossman
            US EPA - NRMRL
                                                                    
This module contains the function saveinpfile() which saves the
data describing a piping network to a file in EPANET's text format.                                    
                                                                    
********************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "hash.h"    
#include "text.h"
#include "types.h"
#include "funcs.h"
#define  EXTERN  extern
#include "vars.h"

/* Defined in enumstxt.h in EPANET.C */
extern char *LinkTxt[];
extern char *FormTxt[];
extern char *StatTxt[];
extern char *FlowUnitsTxt[];
extern char *PressUnitsTxt[];
extern char *ControlTxt[];
extern char *SourceTxt[];
extern char *MixTxt[];
extern char *TstatTxt[];
extern char *RptFlagTxt[];
extern char *SectTxt[];


/*** Updated 6/24/02 ***/
void  savemapinfo(FILE *ftmp)
{
   int   sect,newsect;
   char  *tok; 
   char  line[MAXLINE+1];
   char  s[MAXLINE+1];

   sect = -1;
   rewind(InFile);
   while (fgets(line,MAXLINE,InFile) != NULL)
   {
   /* Skip blank lines & those beginning with a comment */
      strcpy(s,line);
      tok = strtok(line,SEPSTR);
      if (tok == NULL) continue;
      if (*tok == ';') continue;

   /* Check if line begins with a new section heading */
      if (*tok == '[')
      {
         newsect = findmatch(tok,SectTxt);
         if (newsect >= 0)
         {
            sect = newsect;
            if (sect == _END) break;
            switch(sect)
            {
               case _RULES:
               case _COORDS:
               case _VERTICES:
               case _LABELS:
               case _BACKDROP:
               case _TAGS: fwrite(s, sizeof(char), strlen(s), ftmp);
            }
            continue;
         }
         else continue;
      }

   /* Write lines appearing in the section to file */
      switch(sect)
      {
          case _RULES:
          case _COORDS:
          case _VERTICES:
          case _LABELS:
          case _BACKDROP:
          case _TAGS: fwrite(s, sizeof(char), strlen(s), ftmp);
      }
   }
}
/*** End of update ***/


int  saveinpfile(char *fname)
/*
-------------------------------------------------
  Writes network data to text file.
-------------------------------------------------
*/
{
   int   i,j,n;
   double d,kc,ke,km,ucf;

/*** Updated 6/24/02 ***/
   char  s[MAXLINE+1], s1[MAXLINE+1], s2[MAXLINE+1];

   Pdemand demand;
   Psource source;
   FILE  *f;

/*** Updated 6/24/02 ***/
   FILE  *ftmp;

/* Copy [RULES], [COORDS], [VERTICES], [LABELS], [BACKDROP] & [TAGS] */
/* sections from original input file to new input file */

   ftmp = NULL;
   if (InFile)
   {
      ftmp = tmpfile();
      if (ftmp) savemapinfo(ftmp);
   }

/* Open text file */

   if ((f = fopen(fname,"wt")) == NULL)
   {
      if (ftmp) fclose(ftmp);
      return(102);
   }

/*** End of update ***/

/* Write [TITLE] section */

   fprintf(f,"[TITLE]");
   for (i=0; i<3; i++)
   {
      if (strlen(Title[i]) > 0) fprintf(f,"\n%s",Title[i]);
   }

/* Write [JUNCTIONS] section */
/* (Leave demands for [DEMANDS] section) */

   fprintf(f,"\n\n[JUNCTIONS]");
   for (i=1; i<=Njuncs; i++)
      fprintf(f,"\n %-15s %12.2f", Node[i].ID, Node[i].El*Ucf[ELEV]);

/* Write [RESERVOIRS] section */

   fprintf(f,"\n\n[RESERVOIRS]");
   for (i=1; i<=Ntanks; i++)
   {
      if (Tank[i].A == 0.0)
      {
         n = Tank[i].Node;
         sprintf(s," %-15s %12.2f",Node[n].ID, Node[n].El*Ucf[ELEV]);
         if ((j = Tank[i].Pat) > 0)
            sprintf(s1," %-15s",Pattern[j].ID);
         else
            strcpy(s1,"");
         fprintf(f, "\n%s %s", s,s1);
      }
   }

/* Write [TANKS] section */

   fprintf(f,"\n\n[TANKS]");
   for (i=1; i<=Ntanks; i++)
   {
      if (Tank[i].A > 0.0)
      {
         n = Tank[i].Node;
         sprintf(s," %-15s %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f",
            Node[n].ID,
            Node[n].El*Ucf[ELEV],
            (Tank[i].H0 - Node[n].El)*Ucf[ELEV],
            (Tank[i].Hmin - Node[n].El)*Ucf[ELEV],
            (Tank[i].Hmax - Node[n].El)*Ucf[ELEV],
            sqrt(4.0*Tank[i].A/PI)*Ucf[ELEV],
            Tank[i].Vmin*SQR(Ucf[ELEV])*Ucf[ELEV]);
         if ((j = Tank[i].Vcurve) > 0)
            sprintf(s1,"%-15s",Curve[j].ID);
         else
            strcpy(s1,"");
         fprintf(f, "\n%s %s", s,s1);
      }
   }

/* Write [PIPES] section */

   fprintf(f,"\n\n[PIPES]");
   for (i=1; i<=Nlinks; i++)
   {
      if (Link[i].Type <= PIPE)
      {
         d = Link[i].Diam;
         kc = Link[i].Kc;
         if (Formflag == DW) kc = kc*Ucf[ELEV]*1000.0;
         km = Link[i].Km*SQR(d)*SQR(d)/0.02517;

/*** Updated 6/24/02 ***/
         sprintf(s," %-15s %-15s %-15s %12.2f %12.2f",
            Link[i].ID,
            Node[Link[i].N1].ID,
            Node[Link[i].N2].ID,
            Link[i].Len*Ucf[LENGTH],
            d*Ucf[DIAM]);
         if (Formflag == DW) sprintf(s1, "%12.4f %12.4f", kc, km);
         else                sprintf(s1, "%12.2f %12.4f", kc, km);
         if (Link[i].Type == CV) sprintf(s2,"CV");
         else if (Link[i].Stat == CLOSED) sprintf(s2,"CLOSED");
         else strcpy(s2,"");
         fprintf(f,"\n%s %s %s",s,s1,s2);
/*** End of update ***/
      }
   }

/* Write [PUMPS] section */

   fprintf(f, "\n\n[PUMPS]");
   for (i=1; i<=Npumps; i++)
   {
      n = Pump[i].Link;
      sprintf(s," %-15s %-15s %-15s",
         Link[n].ID,
         Node[Link[n].N1].ID,
         Node[Link[n].N2].ID);

   /* Pump has constant power */
      if (Pump[i].Ptype == CONST_HP)
         sprintf(s1, "  POWER %.2f", Link[n].Km);

   /* Pump has a head curve */
      else if ((j = Pump[i].Hcurve) > 0)
         sprintf(s1, "  HEAD %s", Curve[j].ID);

   /* Old format used for pump curve */
      else
      {
         fprintf(f, "\n%s %12.2f %12.2f %12.2f          0.0 %12.2f",s,
                 -Pump[i].H0*Ucf[HEAD],
                 (-Pump[i].H0 - Pump[i].R*pow(Pump[i].Q0,Pump[i].N))*Ucf[HEAD],
                 Pump[i].Q0*Ucf[FLOW],
                 Pump[i].Qmax*Ucf[FLOW]);
         continue;
      }
      strcat(s,s1);

      if ((j = Pump[i].Upat) > 0)
         sprintf(s1,"   PATTERN  %s",Pattern[j].ID);
      else strcpy(s1,"");
      strcat(s,s1);

      if (Link[n].Kc != 1.0)
         sprintf(s1, "  SPEED %.2f", Link[n].Kc);
      else strcpy(s1,"");
      strcat(s,s1);

      fprintf(f,"\n%s",s);
   }

/* Write [VALVES] section */

   fprintf(f, "\n\n[VALVES]");
   for (i=1; i<=Nvalves; i++)
   {
      n = Valve[i].Link;
      d = Link[n].Diam;
      kc = Link[n].Kc;
      if (kc == MISSING) kc = 0.0;
      switch (Link[n].Type)
      {
         case FCV: kc *= Ucf[FLOW]; break;
         case PRV:
         case PSV:
         case PBV: kc *= Ucf[PRESSURE]; break;
      }
      km = Link[n].Km*SQR(d)*SQR(d)/0.02517;

      sprintf(s," %-15s %-15s %-15s %12.2f %5s",
         Link[n].ID,
         Node[Link[n].N1].ID,
         Node[Link[n].N2].ID,
         d*Ucf[DIAM],
         LinkTxt[Link[n].Type]);

      if (Link[n].Type == GPV && (j = ROUND(Link[n].Kc)) > 0)
         sprintf(s1,"%-15s %12.2f", Curve[j].ID, km);
      else sprintf(s1,"%12.2f %12.2f",kc,km);

      fprintf(f, "\n%s %s", s,s1);
   }

/* Write [DEMANDS] section */
   
   fprintf(f, "\n\n[DEMANDS]");

/*** Updated 11/19/01 ***/
   ucf = Ucf[DEMAND];

   for (i=1; i<=Njuncs; i++)
   {
      for (demand = Node[i].D; demand != NULL; demand = demand->next)
      {
         sprintf(s," %-15s %12.2f",Node[i].ID,ucf*demand->Base);
         if ((j = demand->Pat) > 0) sprintf(s1,"   %s",Pattern[j].ID);
         else strcpy(s1,"");
         fprintf(f,"\n%s %s",s,s1);
      }
   }

/* Write [EMITTERS] section */

   fprintf(f, "\n\n[EMITTERS]");
   for (i=1; i<=Njuncs; i++)
   {
      if (Node[i].Ke == 0.0) continue;
      ke = Ucf[FLOW]/pow(Ucf[PRESSURE]*Node[i].Ke,(1.0/Qexp));
      fprintf(f,"\n %-15s %12.2f",Node[i].ID,ke);
   }

/* Write [STATUS] section */

   fprintf(f, "\n\n[STATUS]");
   for (i=1; i<=Nlinks; i++)
   {
      if (Link[i].Type <= PUMP)
      {
         if (Link[i].Stat == CLOSED)
            fprintf(f, "\n %-15s %s",Link[i].ID,StatTxt[CLOSED]);

      /* Write pump speed here for pumps with old-style pump curve input */
         else if (Link[i].Type == PUMP)
         {
            n = PUMPINDEX(i);
            if (
                 Pump[n].Hcurve == 0 &&
                 Pump[n].Ptype != CONST_HP &&
                 Link[i].Kc != 1.0
               )
               fprintf(f, "\n %-15s %-.2f",Link[i].ID, Link[i].Kc);
         }
      }

   /* Write fixed-status PRVs & PSVs (setting = MISSING) */
      else if (Link[i].Kc == MISSING)
      {
         if (Link[i].Stat == OPEN)
            fprintf(f, "\n %-15s %s",Link[i].ID,StatTxt[OPEN]);
         if (Link[i].Stat == CLOSED)
            fprintf(f, "\n%-15s %s",Link[i].ID,StatTxt[CLOSED]);
      }
   }

/* Write [PATTERNS] section */
/* (Use 6 pattern factors per line) */

   fprintf(f, "\n\n[PATTERNS]");
   for (i=1; i<=Npats; i++)
   {
      for (j=0; j<Pattern[i].Length; j++)
      {
        if (j % 6 == 0) fprintf(f,"\n %-15s",Pattern[i].ID);
        fprintf(f," %12.4f",Pattern[i].F[j]);
      }
   }

/* Write [CURVES] section */

   fprintf(f, "\n\n[CURVES]");
   for (i=1; i<=Ncurves; i++)
   {
      for (j=0; j<Curve[i].Npts; j++)
         fprintf(f,"\n %-15s %12.4f %12.4f",
            Curve[i].ID,Curve[i].X[j],Curve[i].Y[j]);
   }

/* Write [CONTROLS] section */

   fprintf(f, "\n\n[CONTROLS]");
   for (i=1; i<=Ncontrols; i++)
   {
   /* Check that controlled link exists */
      if ( (j = Control[i].Link) <= 0) continue;

   /* Get text of control's link status/setting */
      if (Control[i].Setting == MISSING)
         sprintf(s, " LINK %s %s ", Link[j].ID, StatTxt[Control[i].Status]);
      else
      {
         kc = Control[i].Setting;
         switch(Link[j].Type)
         {
            case PRV:
            case PSV:
            case PBV: kc *= Ucf[PRESSURE]; break;
            case FCV: kc *= Ucf[FLOW];     break;
         }
         sprintf(s, " LINK %s %.4f",Link[j].ID, kc);
      }
      
      switch (Control[i].Type)
      {
      /* Print level control */
         case LOWLEVEL:
         case HILEVEL:
            n = Control[i].Node;
            kc = Control[i].Grade - Node[n].El;
            if (n > Njuncs) kc *= Ucf[HEAD];
            else            kc *= Ucf[PRESSURE];
            fprintf(f, "\n%s IF NODE %s %s %.4f", s,
               Node[n].ID, ControlTxt[Control[i].Type], kc);
            break;

      /* Print timer control */
         case TIMER:
            fprintf(f, "\n%s AT %s %.2f HOURS",
               s, ControlTxt[TIMER], Control[i].Time/3600.);
            break;
                         
      /* Print time-of-day control */
         case TIMEOFDAY:
            fprintf(f, "\n%s AT %s %s",
               s, ControlTxt[TIMEOFDAY], clocktime(Atime, Control[i].Time));
            break;
      }
   }            

/* Write [QUALITY] section */
/* (Skip nodes with default quality of 0) */

   fprintf(f, "\n\n[QUALITY]");
   for (i=1; i<=Nnodes; i++)
   {
      if (Node[i].C0 == 0.0) continue;
      fprintf(f, "\n %-15s %12.3f",Node[i].ID,Node[i].C0*Ucf[QUALITY]);
   }
      
/* Write [SOURCES] section */

   fprintf(f, "\n\n[SOURCES]");
   for (i=1; i<=Nnodes; i++)
   {
      source = Node[i].S;
      if (source == NULL) continue;
      sprintf(s," %-15s %-8s %12.2f",
         Node[i].ID,
         SourceTxt[source->Type],
         source->C0);
      if ((j = source->Pat) > 0)
         sprintf(s1,"%s",Pattern[j].ID);
      else strcpy(s1,"");
      fprintf(f,"\n%s %s",s,s1);
   }

/* Write [MIXING] section */

   fprintf(f, "\n\n[MIXING]");
   for (i=1; i<=Ntanks; i++)
   {
      if (Tank[i].A == 0.0) continue;
      fprintf(f, "\n %-15s %-8s %12.4f",
              Node[Tank[i].Node].ID,
              MixTxt[Tank[i].MixModel],
              (Tank[i].V1max/Tank[i].Vmax));
   }

/* Write [REACTIONS] section */

   fprintf(f, "\n\n[REACTIONS]");
   fprintf(f, "\n ORDER  BULK            %-.2f", BulkOrder);
   fprintf(f, "\n ORDER  WALL            %-.0f", WallOrder);
   fprintf(f, "\n ORDER  TANK            %-.2f", TankOrder);
   fprintf(f, "\n GLOBAL BULK            %-.4f", Kbulk*SECperDAY);
   fprintf(f, "\n GLOBAL WALL            %-.4f", Kwall*SECperDAY);
   if (Climit > 0.0) 
   fprintf(f, "\n LIMITING POTENTIAL     %-.4f", Climit);
   if (Rfactor != MISSING && Rfactor != 0.0)
   fprintf(f, "\n ROUGHNESS CORRELATION  %-.4f",Rfactor);
   for (i=1; i<=Nlinks; i++)
   {
      if (Link[i].Type > PIPE) continue;
      if (Link[i].Kb != Kbulk)
         fprintf(f, "\n BULK   %-15s %-.4f",Link[i].ID,Link[i].Kb*SECperDAY);
      if (Link[i].Kw != Kwall)
         fprintf(f, "\n WALL   %-15s %-.4f",Link[i].ID,Link[i].Kw*SECperDAY);
   }
   for (i=1; i<=Ntanks; i++)
   {
      if (Tank[i].A == 0.0) continue;
      if (Tank[i].Kb != Kbulk)
         fprintf(f, "\n TANK   %-15s %-.4f",Node[Tank[i].Node].ID,
            Tank[i].Kb*SECperDAY);
   }

/* Write [ENERGY] section */

   fprintf(f, "\n\n[ENERGY]");
   if (Ecost != 0.0)
   fprintf(f, "\n GLOBAL PRICE        %-.4f", Ecost);
   if (Epat != 0)
   fprintf(f, "\n GLOBAL PATTERN      %s",  Pattern[Epat].ID);
   fprintf(f, "\n GLOBAL EFFIC        %-.2f", Epump);
   fprintf(f, "\n DEMAND CHARGE       %-.4f", Dcost);
   for (i=1; i<=Npumps; i++)
   {
      if (Pump[i].Ecost > 0.0)
         fprintf(f, "\n PUMP %-15s PRICE   %-.4f",
            Link[Pump[i].Link].ID,Pump[i].Ecost);
      if (Pump[i].Epat > 0.0)
         fprintf(f, "\n PUMP %-15s PATTERN %s",
            Link[Pump[i].Link].ID,Pattern[Pump[i].Epat].ID);

/*** Updated 3/1/01 ***/
      if (Pump[i].Ecurve > 0.0)
         fprintf(f, "\n PUMP %-15s EFFIC   %s",
            Link[Pump[i].Link].ID,Curve[Pump[i].Ecurve].ID); 
   }

/* Write [TIMES] section */

   fprintf(f, "\n\n[TIMES]");
   fprintf(f, "\n DURATION            %s",clocktime(Atime,Dur));
   fprintf(f, "\n HYDRAULIC TIMESTEP  %s",clocktime(Atime,Hstep));
   fprintf(f, "\n QUALITY TIMESTEP    %s",clocktime(Atime,Qstep));
   fprintf(f, "\n REPORT TIMESTEP     %s",clocktime(Atime,Rstep));
   fprintf(f, "\n REPORT START        %s",clocktime(Atime,Rstart));
   fprintf(f, "\n PATTERN TIMESTEP    %s",clocktime(Atime,Pstep));
   fprintf(f, "\n PATTERN START       %s",clocktime(Atime,Pstart));
   fprintf(f, "\n RULE TIMESTEP       %s",clocktime(Atime,Rulestep));
   fprintf(f, "\n START CLOCKTIME     %s",clocktime(Atime,Tstart));
   fprintf(f, "\n STATISTIC           %s",TstatTxt[Tstatflag]);

/* Write [OPTIONS] section */

   fprintf(f, "\n\n[OPTIONS]");
   fprintf(f, "\n UNITS               %s", FlowUnitsTxt[Flowflag]);
   fprintf(f, "\n PRESSURE            %s", PressUnitsTxt[Pressflag]);                          
   fprintf(f, "\n HEADLOSS            %s", FormTxt[Formflag]);
   if (DefPat >= 1 && DefPat <= Npats)
   fprintf(f, "\n PATTERN             %s", Pattern[DefPat].ID);
   if (Hydflag == USE)                        
   fprintf(f, "\n HYDRAULICS USE      %s", HydFname);
   if (Hydflag == SAVE)
   fprintf(f, "\n HYDRAULICS SAVE     %s", HydFname);
   if (ExtraIter == -1)
   fprintf(f, "\n UNBALANCED          STOP");
   if (ExtraIter >= 0)
   fprintf(f, "\n UNBALANCED          CONTINUE %d", ExtraIter); 
   if (Qualflag == CHEM)
   fprintf(f, "\n QUALITY             %s %s", ChemName, ChemUnits);
   if (Qualflag == TRACE)
   fprintf(f, "\n QUALITY             TRACE %-15s", Node[TraceNode].ID);
   if (Qualflag == AGE)
   fprintf(f, "\n QUALITY             AGE");
   if (Qualflag == NONE)
   fprintf(f, "\n QUALITY             NONE");
   fprintf(f, "\n DEMAND MULTIPLIER   %-.2f", Dmult);

/*** Updated 11/19/01 ***/
   fprintf(f, "\n EMITTER EXPONENT    %-.2f", 1.0/Qexp);

   fprintf(f, "\n VISCOSITY           %-.4f", Viscos/VISCOS);                                  
   fprintf(f, "\n DIFFUSIVITY         %-.4f", Diffus/DIFFUS);                                  
   fprintf(f, "\n SPECIFIC GRAVITY    %-.4f", SpGrav);                                  
   fprintf(f, "\n TRIALS              %-d",   MaxIter);                                  
   fprintf(f, "\n ACCURACY            %-.8f", Hacc);                                  
   fprintf(f, "\n TOLERANCE           %-.8f", Ctol*Ucf[QUALITY]);

/* Write [REPORT] section */

   fprintf(f, "\n\n[REPORT]");
   fprintf(f, "\n PAGESIZE            %d", PageSize);
   fprintf(f, "\n STATUS              %s", RptFlagTxt[Statflag]);
   fprintf(f, "\n SUMMARY             %s", RptFlagTxt[Summaryflag]);
   fprintf(f, "\n ENERGY              %s", RptFlagTxt[Energyflag]);
   switch (Nodeflag)
   {
      case 0:
      fprintf(f, "\n NODES               NONE");
      break;
      case 1:
      fprintf(f, "\n NODES               ALL");
      break;
      default:
      j = 0;
      for (i=1; i<=Nnodes; i++)
      {
         if (Node[i].Rpt == 1)
         {
            if (j % 5 == 0) fprintf(f, "\n NODES               ");
            fprintf(f, "%s ", Node[i].ID);
            j++;
         }
      }
   }
   switch (Linkflag)
   {
      case 0:
      fprintf(f, "\n LINKS               NONE");
      break;
      case 1:
      fprintf(f, "\n LINKS               ALL");
      break;
      default:
      j = 0;
      for (i=1; i<=Nlinks; i++)
      {
         if (Link[i].Rpt == 1)
         {
            if (j % 5 == 0) fprintf(f, "\n LINKS               ");
            fprintf(f, "%s ", Link[i].ID);
            j++;
         }
      }
   }
   for (i=0; i<MAXVAR; i++)
   {
/*** Updated ********************************************************/         //(2.00.11 - LR)
      if (Field[i].Enabled == TRUE)
      {
         fprintf(f, "\n %-20sPRECISION %d", Field[i].Name, Field[i].Precision);
         if (Field[i].RptLim[LOW] < BIG)
            fprintf(f, "\n %-20sBELOW %.4f", Field[i].Name, Field[i].RptLim[LOW]);
         if (Field[i].RptLim[HI] > -BIG)
            fprintf(f, "\n %-20sABOVE %.4f", Field[i].Name, Field[i].RptLim[HI]);
      }
      else fprintf(f, "\n  %-20sNO", Field[i].Name);
/********************************************************************/
   }
   fprintf(f, "\n");

/*** Updated *****************************************/                        //(2.00.11 - LR)
/* Copy data from scratch file to new input file */
   if (ftmp != NULL)
   {
      fseek(ftmp, 0, SEEK_SET);
      while ( (j = fgetc(ftmp)) != EOF ) fputc(j, f);
      fclose(ftmp);
   } 
/*****************************************************/   

   fprintf(f, "\n\n[END]");
   fclose(f);
   return(0);
}

