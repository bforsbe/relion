/***************************************************************************
 *
 * Author: "Sjors H.W. Scheres"
 * MRC Laboratory of Molecular Biology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This complete copyright notice must be included in any revised version of the
 * source code. Additional authorship citations may be added, but existing
 * author citations must be preserved.
 ***************************************************************************/

#include <src/image.h>
#include <src/metadata_table.h>
#include <src/filename.h>

class star_handler_parameters
{
	public:
   	FileName fn_in, fn_out, fn_compare, fn_label1, fn_label2, fn_label3, select_label;
   	FileName fn_check, fn_operate, fn_operate2, fn_operate3, fn_set;
	RFLOAT eps, select_minval, select_maxval, multiply_by, add_to, center_X, center_Y, center_Z;
	bool do_combine, do_split, do_center, do_random_order;
	long int nr_split, size_split;
	// I/O Parser
	IOParser parser;


	void usage()
	{
		parser.writeUsage(std::cerr);
	}

	void read(int argc, char **argv)
	{

		parser.setCommandLine(argc, argv);

		int general_section = parser.addSection("General options");
	    fn_in = parser.getOption("--i", "Input STAR file ");
	    fn_out = parser.getOption("--o", "Output STAR file ");


		int compare_section = parser.addSection("Compare options");
	    fn_compare = parser.getOption("--compare", "STAR file name to compare the input STAR file with", "");
	    fn_label1 = parser.getOption("--label1", "1st metadata label for the comparison (may be string, int or RFLOAT)", "");
	    fn_label2 = parser.getOption("--label2", "2nd metadata label for the comparison (RFLOAT only) for 2D/3D-distance)", "");
	    fn_label3 = parser.getOption("--label3", "3rd metadata label for the comparison (RFLOAT only) for 3D-distance)", "");
	    eps = textToFloat(parser.getOption("--max_dist", "Maximum distance to consider a match (for int and RFLOAT only)", "0."));

	    int subset_section = parser.addSection("Select options");
	    select_label = parser.getOption("--select", "Metadata label to base output selection on (e.g. rlnCtfFigureOfMerit)", "");
	    select_minval = textToFloat(parser.getOption("--minval", "Minimum acceptable value for this label", "-99999999."));
	    select_maxval = textToFloat(parser.getOption("--maxval", "Maximum acceptable value for this label", "99999999."));

	    int combine_section = parser.addSection("Combine options");
	    do_combine = parser.checkOption("--combine", "Combine input STAR files (multiple individual filenames, all within double-quotes after --i)");
	    fn_check = parser.getOption("--check_duplicates", "MetaDataLabel (for a string only!) to check for duplicates, e.g. rlnImageName", "");

	    int split_section = parser.addSection("Split options");
	    do_split = parser.checkOption("--split", "Split the input STAR file into one or more smaller output STAR files");
	    do_random_order = parser.checkOption("--random_order", "Perform splits on randomised order of the input STAR file");
	    nr_split = textToInteger(parser.getOption("--nr_split", "Split into this many equal-sized STAR files", "-1"));
	    size_split = textToInteger(parser.getOption("--size_split", "AND/OR split into subsets of this many lines", "-1"));

	    int operate_section = parser.addSection("Operate options");
	    fn_operate = parser.getOption("--operate", "Operate on this metadata label", "");
	    fn_operate2 = parser.getOption("--operate2", "Operate also on this metadata label", "");
	    fn_operate3 = parser.getOption("--operate3", "Operate also on this metadata label", "");
	    fn_set = parser.getOption("--set_to", "Set all the values for the --operate label(s) to this value", "");
	    multiply_by = textToFloat(parser.getOption("--multiply_by", "Multiply all the values for the --operate label(s) by this value", "1."));
	    add_to = textToFloat(parser.getOption("--add_to", "Add this value to all the values for the --operate label(s)", "0."));

	    int center_section = parser.addSection("Center options");
	    do_center = parser.checkOption("--center", "Perform centering of particles according to a position in the reference.");
	    center_X = textToFloat(parser.getOption("--center_X", "X-coordinate in the reference to center particles on (in pix)", "0."));
	    center_Y = textToFloat(parser.getOption("--center_Y", "Y-coordinate in the reference to center particles on (in pix)", "0."));
	    center_Z = textToFloat(parser.getOption("--center_Z", "Z-coordinate in the reference to center particles on (in pix)", "0."));

	    // Check for errors in the command-line option
    	if (parser.checkForErrors())
    		REPORT_ERROR("Errors encountered on the command line, exiting...");
	}


	void run()
	{
		int c = 0;
		if (fn_compare != "") c++;
		if (select_label != "") c++;
		if (do_combine) c++;
		if (do_split) c++;
		if (fn_operate != "") c++;
		if (do_center) c++;
		if (c != 1)
			REPORT_ERROR("ERROR: specify (only and at least) one of the following options: --compare, --select, --combine, --split, --operate or --center");


		if (fn_compare != "") compare();
		if (select_label != "") select();
		if (do_combine) combine();
		if (do_split) split();
		if (fn_operate != "") operate();
		if (do_center) center();

		std::cout << " Done!" << std::endl;


	}


	void compare()
	{

	   	MetaDataTable MD1, MD2, MDonly1, MDonly2, MDboth;
		EMDLabel label1, label2, label3;
		MD1.read(fn_in);
		MD2.read(fn_compare);

		label1 = EMDL::str2Label(fn_label1);
		label2 = (fn_label2 == "") ? EMDL_UNDEFINED : EMDL::str2Label(fn_label2);
		label3 = (fn_label3 == "") ? EMDL_UNDEFINED : EMDL::str2Label(fn_label3);


		compareMetaDataTable(MD1, MD2, MDboth, MDonly1, MDonly2, label1, eps, label2, label3);

		std::cout << MDboth.numberOfObjects()  << " entries occur in both input STAR files." << std::endl;
		std::cout << MDonly1.numberOfObjects() << " entries occur only in the 1st input STAR file." << std::endl;
		std::cout << MDonly2.numberOfObjects() << " entries occur only in the 2nd input STAR file." << std::endl;

		MDboth.write(fn_out.insertBeforeExtension("_both"));
		std::cout << " Written: " << fn_out.insertBeforeExtension("_both") << std::endl;
		MDonly1.write(fn_out.insertBeforeExtension("_only1"));
		std::cout << " Written: " << fn_out.insertBeforeExtension("_only1") << std::endl;
		MDonly2.write(fn_out.insertBeforeExtension("_only2"));
		std::cout << " Written: " << fn_out.insertBeforeExtension("_only2") << std::endl;
	}

	void select()
	{

		MetaDataTable MDin, MDout;

		if (fn_in.contains("_model.star"))
		{
			MDin.read(fn_in, "model_classes");
		}
		else
		{
			MDin.read(fn_in);
		}

		MDout = subsetMetaDataTable(MDin, EMDL::str2Label(select_label), select_minval, select_maxval);

		MDout.write(fn_out);
		std::cout << " Written: " << fn_out << std::endl;

	}

	void combine()
	{

		std::vector<FileName> fns_in;
		std::vector<std::string> words;
		tokenize(fn_in, words);
		for (int iword = 0; iword < words.size(); iword++)
		{
			FileName fnt = words[iword];
			fnt.globFiles(fns_in, false);
		}

		MetaDataTable MDout;
		std::vector<MetaDataTable> MDsin;
		for (int i = 0; i < fns_in.size(); i++)
		{
			MetaDataTable MDin;
			MDin.read(fns_in[i]);
			MDsin.push_back(MDin);
		}

		MDout = combineMetaDataTables(MDsin);

		if (fn_check != "")
		{
			EMDLabel label = EMDL::str2Label(fn_check);
			if (!MDout.containsLabel(label))
				REPORT_ERROR("ERROR: the output file does not contain the label to check for duplicates. Is it present in all input files?");

			/// Don't want to mess up original order, so make a MDsort with only that label...
			FileName fn_this, fn_prev = "";
			MetaDataTable MDsort;
			FOR_ALL_OBJECTS_IN_METADATA_TABLE(MDout)
			{
				MDout.getValue(label, fn_this);
				MDsort.addObject();
				MDsort.setValue(label, fn_this);
			}
			// sort on the label
			MDsort.newSort(label);
			long int nr_duplicates = 0;
			FOR_ALL_OBJECTS_IN_METADATA_TABLE(MDsort)
			{
				MDsort.getValue(label, fn_this);
				if (fn_this == fn_prev)
				{
					nr_duplicates++;
					std::cerr << " WARNING: duplicate entry: " << fn_this << std::endl;
				}
				fn_prev = fn_this;
			}

			if (nr_duplicates > 0)
				std::cerr << " WARNING: Total number of duplicate "<< fn_check << " entries: " << nr_duplicates << std::endl;
		}

		MDout.write(fn_out);
		std::cout << " Written: " << fn_out << std::endl;

	}

	void split()
	{

		MetaDataTable MD;
		MD.read(fn_in);

		// Randomise if neccesary
		if (do_random_order)
		{
			MD.randomiseOrder();
		}

		long int n_obj = MD.numberOfObjects();

		if (nr_split < 0 && size_split < 0)
		{
			REPORT_ERROR("ERROR: nr_split and size_split are both zero. Set at least one of them to be positive.");
		}
		else if (nr_split < 0 && size_split > 0)
		{
			if (size_split > n_obj)
			{
				std::cout << " Nothing to do, as size_split is set to a larger value than the number of input images..." << std::endl;
				return;
			}
			nr_split = n_obj / size_split;
		}
		else if (nr_split > 0 && size_split < 0)
		{
			size_split = CEIL(1. * n_obj / nr_split);
		}

		std::vector<MetaDataTable > MDouts;
		MDouts.resize(nr_split);

		long int n = 0;
		FOR_ALL_OBJECTS_IN_METADATA_TABLE(MD)
		{
			int my_split = n / size_split;
			if (my_split < nr_split)
			{
				MDouts[my_split].addObject(MD.getObject(current_object));
			}
			else
			{
				break;
			}
			n++;
		}

		for (int isplit = 0; isplit < nr_split; isplit ++)
		{
			FileName fnt = fn_out.insertBeforeExtension("_split"+integerToString(isplit+1,3));
			MDouts[isplit].write(fnt);
			std::cout << " Written: " <<fnt << " with " << MDouts[isplit].numberOfObjects() << " objects." << std::endl;
		}


	}

	void operate()
	{
		EMDLabel label1, label2, label3;
		label1 = EMDL::str2Label(fn_operate);
		if (fn_operate2 != "")
		{
			label2 = EMDL::str2Label(fn_operate2);
		}
		if (fn_operate3 != "")
		{
			label3 = EMDL::str2Label(fn_operate3);
		}

		MetaDataTable MD;
		MD.read(fn_in);

		FOR_ALL_OBJECTS_IN_METADATA_TABLE(MD)
		{

			if (EMDL::isDouble(label1))
			{
				RFLOAT val;
				if (fn_set != "")
				{
					val = textToFloat(fn_set);
					MD.setValue(label1, val);
					if (fn_operate2 != "") MD.setValue(label2, val);
					if (fn_operate3 != "") MD.setValue(label3, val);
				}
				else if (multiply_by != 1. || add_to != 0.)
				{
					MD.getValue(label1, val);
					val = multiply_by * val + add_to;
					MD.setValue(label1, val);
					if (fn_operate2 != "")
					{
						MD.getValue(label2, val);
						val = multiply_by * val + add_to;
						MD.setValue(label2, val);
					}
					if (fn_operate3 != "")
					{
						MD.getValue(label3, val);
						val = multiply_by * val + add_to;
						MD.setValue(label3, val);
					}
				}
			}
			else if (EMDL::isInt(label1))
			{
				int val;
				if (fn_set != "")
				{
					val = textToInteger(fn_set);
					MD.setValue(label1, val);
					if (fn_operate2 != "") MD.setValue(label2, val);
					if (fn_operate3 != "") MD.setValue(label3, val);
				}
				else if (multiply_by != 1. || add_to != 0.)
				{
					MD.getValue(label1, val);
					val = multiply_by * val + add_to;
					MD.setValue(label1, val);
					if (fn_operate2 != "")
					{
						MD.getValue(label2, val);
						val = multiply_by * val + add_to;
						MD.setValue(label2, val);
					}
					if (fn_operate3 != "")
					{
						MD.getValue(label3, val);
						val = multiply_by * val + add_to;
						MD.setValue(label3, val);
					}
				}

			}
			else if (EMDL::isString(label1))
			{
				if (fn_set != "")
				{
					MD.setValue(label1, fn_set);
					if (fn_operate2 != "") MD.setValue(label2, fn_set);
					if (fn_operate3 != "") MD.setValue(label3, fn_set);
				}
				else if (multiply_by != 1. || add_to != 0.)
				{
					REPORT_ERROR("ERROR: cannot multiply_by or add_to a string!");
				}
			}
			else if (EMDL::isBool(label1))
			{
				REPORT_ERROR("ERROR: cannot operate on a boolean!");
			}

		}

		MD.write(fn_out);
		std::cout << " Written: " << fn_out << std::endl;

	}

	void center()
	{
		MetaDataTable MD;
		MD.read(fn_in);
		bool do_contains_xy = (MD.containsLabel(EMDL_ORIENT_ORIGIN_X) && MD.containsLabel(EMDL_ORIENT_ORIGIN_Y));
		bool do_contains_z = (MD.containsLabel(EMDL_ORIENT_ORIGIN_Z));

		if (!do_contains_xy)
		{
			REPORT_ERROR("ERROR: input STAR file does not contain rlnOriginX/Y for re-centering.");
		}

		Matrix1D<RFLOAT> my_center(3);
		XX(my_center) = center_X;
		YY(my_center) = center_Y;
		ZZ(my_center) = center_Z;

		FOR_ALL_OBJECTS_IN_METADATA_TABLE(MD)
		{
			Matrix1D<RFLOAT> my_projected_center(3);
			Matrix2D<RFLOAT> A3D;
			RFLOAT xoff, yoff, zoff, rot, tilt, psi;

			MD.getValue(EMDL_ORIENT_ORIGIN_X, xoff);
			MD.getValue(EMDL_ORIENT_ORIGIN_Y, yoff);
			MD.getValue(EMDL_ORIENT_ROT, rot);
			MD.getValue(EMDL_ORIENT_TILT, tilt);
			MD.getValue(EMDL_ORIENT_PSI, psi);

			// Project the center-coordinates
			Euler_angles2matrix(rot, tilt, psi, A3D, false);
			my_projected_center = A3D * my_center;

			xoff -= XX(my_projected_center);
			yoff -= YY(my_projected_center);

			// Set back the new centers
			MD.setValue(EMDL_ORIENT_ORIGIN_X, xoff);
			MD.setValue(EMDL_ORIENT_ORIGIN_Y, yoff);

			// also allow 3D data (subtomograms)
			if (do_contains_z)
			{
				MD.getValue(EMDL_ORIENT_ORIGIN_Z, zoff);
				zoff -= ZZ(my_projected_center);
				MD.setValue(EMDL_ORIENT_ORIGIN_Z, zoff);
			}
		}


		MD.write(fn_out);
		std::cout << " Written: " << fn_out << std::endl;


	}


};


int main(int argc, char *argv[])
{
	star_handler_parameters prm;

	try
    {

		prm.read(argc, argv);

		prm.run();

    }
    catch (RelionError XE)
    {
        std::cerr << XE;
        //prm.usage();
        exit(1);
    }
    return 0;
}

